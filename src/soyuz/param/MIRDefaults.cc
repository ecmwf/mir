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

#include "soyuz/param/MIRDefaults.h"


namespace mir {
namespace param {


MIRDefaults::MIRDefaults() {
    // For demo only:

    defaults_["logic"] = "mars";
    defaults_["method"] = "bilinear";

}


MIRDefaults::~MIRDefaults() {
}


void MIRDefaults::print(std::ostream& out) const {
    out << "MIRDefaults[";
    const char* sep = "";
    for(std::map<std::string, std::string>::const_iterator j = defaults_.begin(); j != defaults_.end(); ++j) {
        out << sep;
        out << (*j).first << "=" << (*j).second;
        sep = ",";
    }
    out << "]";
}


bool MIRDefaults::get(const std::string& name, std::string& value) const {
    std::map<std::string, std::string>::const_iterator j = defaults_.find(name);
    if(j != defaults_.end()) {
        value = (*j).second;
        return true;
    }
    return false;
}


}  // namespace param
}  // namespace mir

