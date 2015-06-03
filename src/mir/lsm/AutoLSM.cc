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
#include "mir/lsm/TenMinutesLSM.h"
#include "mir/lsm/MappedMask.h"

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
                      const atlas::Grid &grid,
                      const std::string& which) const {

    // Mask* mask = new TenMinutesLSM(name, param, grid, which);
    Mask* mask = new MappedMask(name, param, grid, which);
    // Mask* mask = new GribFileLSM(name, path(param), param, grid, which);

    eckit::Log::info() << "AutoLSM::create => " << *mask << std::endl;
    return mask;
}

std::string AutoLSM::cacheKey(const std::string &name,
                              const param::MIRParametrisation &param,
                              const atlas::Grid &grid,
                              const std::string& which) const {
    eckit::MD5 md5;
    GribFileLSM::hashCacheKey(md5, path(param), param, grid, which); // We need to take the lsm interpolation method into account
    return "auto." + md5.digest();
}


namespace {
static AutoLSM input("auto.input");
static AutoLSM output("auto.output");

}


}  // namespace logic
}  // namespace mir

