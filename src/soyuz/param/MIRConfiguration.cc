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

#include "soyuz/param/MIRConfiguration.h"


namespace mir {
namespace param {


MIRConfiguration::MIRConfiguration() {
    // TODO: Read a file
}


MIRConfiguration::~MIRConfiguration() {
}


void MIRConfiguration::print(std::ostream& out) const {
    out << "MIRConfiguration[...]";
}


bool MIRConfiguration::get(const std::string& name, std::string& value) const {
    std::map<std::string, std::string>::const_iterator j = settings_.find(name);
    if(j != settings_.end()) {
        value = (*j).second;
        return true;
    }
    return false;
}


}  // namespace param
}  // namespace mir

