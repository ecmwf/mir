// File MIRConfiguration.cc
// Baudouin Raoult - (c) ECMWF Apr 15

#include "MIRConfiguration.h"
#include <iostream>

MIRConfiguration::MIRConfiguration()
{
    // TODO: Read a file
}

MIRConfiguration::~MIRConfiguration()
{
}


void MIRConfiguration::print(std::ostream& out) const
{
    out << "MIRConfiguration[...]";
}

bool MIRConfiguration::get(const std::string& name, std::string& value) const {
    return false;
}
