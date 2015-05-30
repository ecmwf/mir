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


#include "GribFileLSM.h"

#include "eckit/filesystem/PathName.h"

#include "mir/input/GribFileInput.h"
#include "mir/data/MIRField.h"
#include "mir/method/Method.h"
#include "mir/param/RuntimeParametrisation.h"
#include "mir/repres/Representation.h"
#include "mir/util/Compare.h"

#include "atlas/Grid.h"

namespace mir {
namespace lsm {

GribFileLSM::GribFileLSM(const std::string &name, const eckit::PathName &path,
                         const param::MIRParametrisation &parametrisation,
                         const atlas::Grid &grid,
                         const std::string &which):
    Mask(name),
    path_(path) {

    // WARNING: don't store the grid, it won't be there later if this
    // object is cached


    eckit::Log::info() << "GribFileLSM loading " << path_ << std::endl;

    mir::input::GribFileInput file( path_ );
    mir::input::MIRInput &input = file;

    ASSERT(file.next());
    std::auto_ptr<data::MIRField> field(input.field());

    param::RuntimeParametrisation runtime(parametrisation);
    runtime.set("lsm", false);

    std::string interpolation;
    if (!parametrisation.get("lsm.interpolation" + which, interpolation)) {
        if (!parametrisation.get("lsm.interpolation", interpolation)) {
            throw eckit::SeriousBug("Not interpolation method defined for land sea mask");
        }
    }

    std::auto_ptr< method::Method > method(method::MethodFactory::build(interpolation, runtime));
    eckit::Log::info() << "LSM interpolation method is " << *method << std::endl;

    std::auto_ptr<atlas::Grid> gin(field->representation()->atlasGrid());

    method->execute(*field, *gin, grid);

    double threshold;
    ASSERT(parametrisation.get("lsm.value.threshold", threshold));
    const util::compare::is_greater_equal_fn< double > check_lsm(threshold);

    ASSERT(!field->hasMissing());
    ASSERT(field->dimensions() == 1);

    const std::vector< double > &values = field->values(0);
    mask_.resize(values.size());
    std::transform(values.begin(), values.end(), mask_.begin(), check_lsm);

}

GribFileLSM::~GribFileLSM() {
}

void GribFileLSM::hash(eckit::MD5 &md5) const {
    Mask::hash(md5);
    md5.add(path_.asString());
}

void GribFileLSM::print(std::ostream &out) const {
    out << "GribFileLSM[name=" << name_ << ",path=" << path_ << "]";
}

void GribFileLSM::hashCacheKey(eckit::MD5 &md5, const eckit::PathName &path,
                               const param::MIRParametrisation &parametrisation,
                               const atlas::Grid &grid,
                               const std::string &which) {

    std::string interpolation;
    if (!parametrisation.get("lsm.interpolation" + which, interpolation)) {
        if (!parametrisation.get("lsm.interpolation", interpolation)) {
            throw eckit::SeriousBug("Not interpolation method defined for land sea mask");
        }
    }

    md5 << path.asString();
    md5 << interpolation;
    md5 << grid;
}


const std::vector<bool> &GribFileLSM::mask() const {
    return mask_;
}

//-----------------------------------------------------------------------------


}  // namespace logic
}  // namespace mir

