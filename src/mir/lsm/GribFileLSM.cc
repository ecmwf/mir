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
/// @date Apr 2015


#include "eckit/exception/Exceptions.h"

#include "mir/lsm/GribFileLSM.h"
#include "mir/input/GribFileInput.h"
#include "mir/data/MIRField.h"
#include "mir/method/Method.h"
#include "mir/param/RuntimeParametrisation.h"
#include "mir/repres/Representation.h"

#include "atlas/Grid.h"

namespace mir {
namespace lsm {

GribFileLSM::GribFileLSM(const std::string &name, const std::string &key, const param::MIRParametrisation &param,
                         const atlas::Grid &grid):

    Mask(name, key) {
    eckit::PathName path("~mir/etc/lsm.N640.grib");
    init(param, grid, path);
}

GribFileLSM::GribFileLSM(const std::string &name, const std::string &key, const param::MIRParametrisation &param,
                         const atlas::Grid &grid, const std::string& path):
    Mask(name, key + path) {
    init(param, grid, path);
}

void GribFileLSM::init(const param::MIRParametrisation &param, const atlas::Grid &grid, const std::string& path) {
    // WARNING: the atlas::Grid will not exist after

    eckit::Log::info() << "GribFileLSM loading " << path << std::endl;

    mir::input::GribFileInput file("~mir/etc/lsm.N640.grib");
    mir::input::MIRInput &input = file;

    ASSERT(file.next());
    field_.reset(input.field());

    param::RuntimeParametrisation runtime(param);
    // Hide the paramID so we don't confuse this LSM with interpolating an LSM from MARS and create an infinite recurrsion

    runtime.set("paramId", -1L);
    runtime.hide("lsm.input");
    runtime.hide("lsm.output");
    runtime.hide("lsm");

    std::string interpolation;
    ASSERT(runtime.get("interpolation", interpolation));

    std::auto_ptr< method::Method > method(method::MethodFactory::build(interpolation, runtime));
    eckit::Log::info() << "LSM interpolation method is " << *method << std::endl;

    std::auto_ptr<atlas::Grid> gin(field_->representation()->atlasGrid());

    method->execute(*field_, *gin, grid);

    field_->representation(0); // This should not be used by users of the LSM



}

GribFileLSM::~GribFileLSM() {
}

void GribFileLSM::print(std::ostream &out) const {
    out << "GribFileLSM[name=" << name_ << ",key=" << key_ << "]";
}


//-----------------------------------------------------------------------------


}  // namespace logic
}  // namespace mir

