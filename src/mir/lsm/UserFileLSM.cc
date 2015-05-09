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

#include "mir/lsm/UserFileLSM.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/lsm/GribFileLSM.h"
#include "atlas/Grid.h"


namespace mir {
namespace lsm {


UserFileLSM::UserFileLSM(const std::string &name, const std::string& which):
    LSMChooser(name), which_(which) {
}


UserFileLSM::~UserFileLSM() {
}

void UserFileLSM::print(std::ostream& out) const {
    out << "UserFileLSM[" << name_ << ",which=" << which_ << "]";
}

LandSeaMask *UserFileLSM::create(const std::string &name, const std::string &key,
                                 const param::MIRParametrisation &param, const atlas::Grid &grid) const {

    std::string path;

    if(!param.get("lsm.path." + which_, path)) {
        if(!param.get("lsm.path", path)) {
            eckit::StrStream os;
            os << *this << " no path specified" << eckit::StrStream::ends;
            throw eckit::UserError(os);
        }
    }

    return new GribFileLSM(name, key, param, grid, path);
}


namespace {
static UserFileLSM input("file.input", "input");
static UserFileLSM output("file.output", "output");

}


}  // namespace logic
}  // namespace mir

