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


#include "mir/lsm/UserFileLSM.h"

#include <iostream>
#include "atlas/grid/Grid.h"
#include "eckit/exception/Exceptions.h"
#include "eckit/utils/MD5.h"
#include "mir/lsm/GribFileLSM.h"
#include "mir/param/MIRParametrisation.h"


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


std::string UserFileLSM::path(const param::MIRParametrisation &param) const {
    std::string path;
    if (!param.get("lsm-file-" + which_, path)) {
        if (!param.get("lsm-file", path)) {
            std::ostringstream os;
            os << *this << " no path specified";
            throw eckit::UserError(os.str());
        }
    }
    return path;
}


Mask *UserFileLSM::create(const std::string &name,
                          const param::MIRParametrisation &param,
                          const atlas::grid::Grid &grid,
                          const std::string& which) const {
    return new GribFileLSM(name, path(param), param, grid, which);
}


std::string UserFileLSM::cacheKey(const std::string &name,
                                  const param::MIRParametrisation &param,
                                  const atlas::grid::Grid &grid,
                                  const std::string& which) const {

    eckit::MD5 md5;
    GribFileLSM::hashCacheKey(md5, path(param), param, grid, which); // We need to take the lsm interpolation method into account

    return "file." + md5.digest();
}


namespace {
static UserFileLSM input("file-input", "input");
static UserFileLSM output("file-output", "output");
}


}  // namespace lsm
}  // namespace mir

