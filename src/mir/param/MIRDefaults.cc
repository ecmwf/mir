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


#include <iostream>
#include <limits>
#include "mir/param/MIRDefaults.h"


namespace mir {
namespace param {


MIRDefaults::MIRDefaults() {
    // For demo only:

    set("logic", "mars");
    set("interpolation", "finite-element"); // The word 'method' is used in grib
    set("epsilon", std::numeric_limits<double>::epsilon());
    set("nclosest", 4L);
}


MIRDefaults::~MIRDefaults() {
}


void MIRDefaults::print(std::ostream& out) const {
    out << "MIRDefaults[";
    SimpleParametrisation::print(out);
    out << "]";
}


}  // namespace param
}  // namespace mir

