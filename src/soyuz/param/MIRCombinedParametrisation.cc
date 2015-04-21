// File MIRCombinedParametrisation.cc
// Baudouin Raoult - (c) ECMWF Apr 15

#include "soyuz/param/MIRCombinedParametrisation.h"
#include <iostream>

MIRCombinedParametrisation::MIRCombinedParametrisation(const MIRParametrisation& user,
        const MIRParametrisation& metadata,
        const MIRParametrisation& configuration,
        const MIRParametrisation& defaults):
    user_(user),
    metadata_(metadata),
    configuration_(configuration),
    defaults_(defaults) {
}

MIRCombinedParametrisation::~MIRCombinedParametrisation() {
}


void MIRCombinedParametrisation::print(std::ostream& out) const {
    out << "MIRCombinedParametrisation[user="
        << user_ <<
        ",metadata=" << metadata_ <<
        ",configuration=" << configuration_ <<
        ",defaults=" << defaults_ <<"]";
}

bool MIRCombinedParametrisation::get(const std::string& name, std::string& value) const {

    if(name.find("user.") == 0) {
        return user_.get(name.substr(5), value);
    }

    if(name.find("field.") == 0) {
        return metadata_.get(name.substr(6), value);
    }

    // This could be a loop
    if(user_.get(name, value)) return true;
    if(metadata_.get(name, value)) return true;
    if(configuration_.get(name, value)) return true;
    if(defaults_.get(name, value)) return true;

    return false;
}

