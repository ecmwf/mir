// File MARSLogic.cc
// Baudouin Raoult - (c) ECMWF Apr 15

#include "MARSLogic.h"

#include <iostream>

MARSLogic::MARSLogic(const MIRParametrisation &parametrisation):
    ECMWFLogic(parametrisation)
{

}

MARSLogic::~MARSLogic() {
}

void MARSLogic::print(std::ostream &out) const {
    out << "MARSLogic[]";
}


static MIRLogicBuilder<MARSLogic> marsGrid("mars");
