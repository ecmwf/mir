/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 *
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/lsm/GribFileMask.h"

#include <algorithm>
#include <memory>
#include <sstream>

#include "eckit/utils/MD5.h"

#include "mir/action/context/Context.h"
#include "mir/data/MIRField.h"
#include "mir/input/GribFileInput.h"
#include "mir/method/Method.h"
#include "mir/param/RuntimeParametrisation.h"
#include "mir/repres/Representation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"
#include "mir/util/MIRStatistics.h"


namespace mir {
namespace lsm {


GribFileMask::GribFileMask(const eckit::PathName& path, const param::MIRParametrisation& parametrisation,
                           const repres::Representation& representation, const std::string& which) :
    path_(path) {

    // WARNING: don't store the grid, it won't be there later if this
    // object is cached


    Log::debug() << "GribFileMask loading " << path_ << std::endl;

    mir::input::GribFileInput file(path_);
    const mir::input::MIRInput& input = file;

    ASSERT(file.next());
    data::MIRField field = input.field();

    param::RuntimeParametrisation runtime(parametrisation);
    runtime.set("lsm", false);

    std::string interpolation;
    if (!parametrisation.get("lsm-interpolation-" + which, interpolation)) {
        if (!parametrisation.get("lsm-interpolation", interpolation)) {
            throw exception::SeriousBug("No interpolation method defined for land-sea mask");
        }
    }

    std::unique_ptr<method::Method> method(method::MethodFactory::build(interpolation, runtime));
    Log::debug() << "LSM interpolation method is " << *method << std::endl;

    if (!(field.representation()->isGlobal())) {
        std::ostringstream oss;
        oss << "LSM file '" << path_ << "' should be global";
        throw exception::UserError(oss.str());
    }

    util::MIRStatistics dummy;  // TODO: use the global one
    context::Context ctx(field, dummy);
    method->execute(ctx, *field.representation(), representation);

    double threshold;
    if (!parametrisation.get("lsm-value-threshold-" + which, threshold)) {
        ASSERT(parametrisation.get("lsm-value-threshold", threshold));
    }


    ASSERT(!ctx.field().hasMissing());
    ASSERT(ctx.field().dimensions() == 1);

    const MIRValuesVector& values = ctx.field().values(0);
    mask_.resize(values.size());

    /// Compare values inequality, "is greater or equal to"
    std::transform(values.begin(), values.end(), mask_.begin(), [&](double value) { return value >= threshold; });
}


void GribFileMask::hash(eckit::MD5& md5) const {
    Mask::hash(md5);
    md5.add(path_.asString());
}


void GribFileMask::print(std::ostream& out) const {
    out << "GribFileMask[path=" << path_ << "]";
}


bool GribFileMask::active() const {
    return true;
}


const std::vector<bool>& GribFileMask::mask() const {
    return mask_;
}


}  // namespace lsm
}  // namespace mir
