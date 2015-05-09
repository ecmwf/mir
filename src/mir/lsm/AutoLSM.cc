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

Mask *AutoLSM::create(const std::string &name,
                      const std::string &key,
                      const param::MIRParametrisation &parametrisation,
                      const atlas::Grid &grid) const {
    return new GribFileLSM(name, key, parametrisation, grid);
}


namespace {
static AutoLSM input("auto.input");
static AutoLSM output("auto.output");

}


}  // namespace logic
}  // namespace mir

