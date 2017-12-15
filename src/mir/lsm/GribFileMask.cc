/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Pedro Maciel
/// @author Tiago Quintino
/// @date Apr 2015


#include "mir/lsm/GribFileMask.h"

#include "eckit/exception/Exceptions.h"
#include "eckit/memory/ScopedPtr.h"
#include "eckit/utils/MD5.h"

#include "mir/action/context/Context.h"
#include "mir/config/LibMir.h"
#include "mir/data/MIRField.h"
#include "mir/input/GribFileInput.h"
#include "mir/method/Method.h"
#include "mir/param/RuntimeParametrisation.h"
#include "mir/repres/Representation.h"
#include "mir/util/MIRStatistics.h"


namespace mir {
namespace lsm {


GribFileMask::GribFileMask(const eckit::PathName& path,
                           const param::MIRParametrisation& parametrisation,
                           const repres::Representation& representation,
                           const std::string& which) :
    Mask(path, parametrisation, representation, which) {
}


GribFileMask::~GribFileMask() {
}


void GribFileMask::hash(eckit::MD5& md5) const {
    Mask::hash(md5);
    md5.add("GribFileMask");
    md5.add(lsmInterpolation());
    md5.add(lsmValueThreshold());
}


void GribFileMask::print(std::ostream& out) const {
    out << "GribFileMask["
        <<  "interpolation=" << lsmInterpolation()
        << ",threshold=" << lsmValueThreshold()
        << ",";
    Mask::print(out);
    out << "]";
}


std::string GribFileMask::lsmInterpolation() const {
    ASSERT(!which_.empty());
    std::string interpolation;
    if (!parametrisation_.get("lsm-interpolation-" + which_, interpolation)) {
        if (!parametrisation_.get("lsm-interpolation", interpolation)) {
            throw eckit::SeriousBug("GribFileMask: no interpolation method defined for land-sea mask");
        }
    }

    ASSERT(!interpolation.empty());
    return interpolation;
}


double GribFileMask::lsmValueThreshold() const {
    double threshold;
    if (!parametrisation_.get("lsm-value-threshold-" + which_, threshold)) {
        ASSERT(parametrisation_.get("lsm-value-threshold", threshold));
    }
    if (threshold < 0.) {
        throw eckit::UserError("GribFileMask: land-sea mask value threshold must be positive (" + std::to_string(threshold) + ")");
    }
    return threshold;
}


bool GribFileMask::active() const {
    return true;
}


const std::vector<bool>& GribFileMask::mask() const {

    // Lazy loading
    if (!mask_.empty()) {
        eckit::Log::debug<LibMir>() << "GribFileMask: passive loading" << std::endl;
        return mask_;
    }


    // WARNING: don't store the grid, it won't be there later if this
    // object is cached


    eckit::Log::debug<LibMir>() << "GribFileMask: loading " << path_ << std::endl;

    mir::input::GribFileInput file( path_ );
    mir::input::MIRInput& input = file;

    ASSERT(file.next());
    data::MIRField field = input.field();

    if (!(field.representation()->isGlobal())) {
        throw eckit::UserError("GribFileMask: file '" + path_ + "' should be global");
    }

    ASSERT(!field.hasMissing());
    ASSERT(field.dimensions() == 1);



    param::RuntimeParametrisation runtime(parametrisation_);
    runtime.set("lsm", false);

    const std::string interpolation = lsmInterpolation();
    eckit::ScopedPtr< method::Method > method(method::MethodFactory::build(interpolation, runtime));

    {
        eckit::Timer("GribFileMask: interpolation");

        const double threshold = lsmValueThreshold();
        eckit::Log::debug<LibMir>() << "GribFileMask: interpolation method: " << *method << std::endl;
        eckit::Log::debug<LibMir>() << "GribFileMask: value threshold: " << threshold << std::endl;

        util::MIRStatistics dummy; // TODO: use the global one
        context::Context ctx(field, dummy);
        method->execute(ctx, *field.representation(), representation_);

        /// Apply threshold, "is greater or equal to"
        const std::vector< double >& values = ctx.field().values(0);
        mask_.resize(values.size());
        std::transform(values.begin(), values.end(), mask_.begin(), [&](double value) { return value >= threshold; });
    }

    ASSERT(mask_.size());
    return mask_;
}


}  // namespace lsm
}  // namespace mir

