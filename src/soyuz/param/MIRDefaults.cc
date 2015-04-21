// File MIRDefaults.cc
// Baudouin Raoult - (c) ECMWF Apr 15

#include "soyuz/param/MIRDefaults.h"
#include <iostream>

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
