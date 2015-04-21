// File MIRParametrisation.cc
// Baudouin Raoult - (c) ECMWF Apr 15

#include "soyuz/param/MIRParametrisation.h"

MIRParametrisation::MIRParametrisation() {
}

MIRParametrisation::~MIRParametrisation() {
}

bool MIRParametrisation::has(const std::string& name) const {
    std::string ignore;
    return get(name, ignore);
}