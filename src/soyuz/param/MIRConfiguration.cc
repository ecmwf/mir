// File MIRConfiguration.cc
// Baudouin Raoult - (c) ECMWF Apr 15

#include "soyuz/param/MIRConfiguration.h"
#include <iostream>

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
