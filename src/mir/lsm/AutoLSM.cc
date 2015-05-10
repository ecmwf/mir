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



#include "mir/lsm/AutoLSM.h"

#include <iostream>
#include "mir/lsm/GribFileLSM.h"
#include "eckit/utils/MD5.h"
#include "atlas/Grid.h"

namespace mir {
namespace lsm {


AutoLSM::AutoLSM(const std::string &name):
    LSMChooser(name) {
}


AutoLSM::~AutoLSM() {
}

void AutoLSM::print(std::ostream& out) const {
    out << "AutoLSM[" << name_ << "]";
}

std::string AutoLSM::path(const param::MIRParametrisation &parametrisation) const {
    // TODO: Implement clever selection
    return  "~mir/etc/lsm.N640.grib";
}

Mask *AutoLSM::create(const std::string &name,
                      const param::MIRParametrisation &param,
                      const atlas::Grid &grid) const {
    return new GribFileLSM(name, path(param), param, grid);
}

std::string AutoLSM::cacheKey(const std::string &name,
                              const param::MIRParametrisation &param,
                              const atlas::Grid &grid) const {
    eckit::MD5 md5;
    md5 << path(param)
        << grid;

    return "auto." + md5.digest();
}


namespace {
static AutoLSM input("auto.input");
static AutoLSM output("auto.output");

}


}  // namespace logic
}  // namespace mir

