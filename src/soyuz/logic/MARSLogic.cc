// File MARSLogic.cc
// Baudouin Raoult - (c) ECMWF Apr 15

#include "soyuz/logic/MARSLogic.h"
#include "soyuz/param/MIRParametrisation.h"

#include <iostream>

MARSLogic::MARSLogic(const MIRParametrisation &parametrisation):
    MIRLogic(parametrisation)
{

}

MARSLogic::~MARSLogic() {
}

void MARSLogic::print(std::ostream &out) const {
    out << "MARSLogic[]";
}

void MARSLogic::prepare(std::vector<std::auto_ptr<Action> > &actions) const {
    // All the nasty logic goes there

    std::string ignore;

    if (parametrisation_.get("field.spherical", ignore)) {
        if (parametrisation_.get("user.truncation", ignore)) {
            add(actions, "transform.sh2sh");
        }
    }

    if (parametrisation_.get("field.spherical", ignore)) {
        if (parametrisation_.get("user.grid", ignore)) {
            add(actions, "transform.sh2grid");
        }
    }

    if (parametrisation_.get("field.gridded", ignore)) {
        if (parametrisation_.get("user.grid", ignore)) {
            add(actions, "interpolate.grid2grid");
        }
    }

    if (parametrisation_.get("user.area", ignore)) {
        add(actions, "crop.area");
    }

    if (parametrisation_.get("user.bitmap", ignore)) {
        add(actions, "filter.bitmap");
    }

    if (parametrisation_.get("user.frame", ignore)) {
        add(actions, "filter.frame");
    }
}


static MIRLogicBuilder<MARSLogic> marsGrid("mars");
