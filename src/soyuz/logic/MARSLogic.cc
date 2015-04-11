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

    if (parametrisation_.has("field.spherical")) {
        if (parametrisation_.has("user.truncation")) {
            add(actions, "transform.sh2sh");
        }
    }

    // Maybe we need different actions
    // user.grid => transform.sh2grid
    // user.reduced => transform.sh2reduced
    // user.regular => transform.sh2regular
    if (parametrisation_.has("field.spherical")) {
        if (parametrisation_.has("user.grid") ||
            parametrisation_.has("user.reduced") ||
            parametrisation_.has("user.regular"))
        {
            add(actions, "transform.sh2grid");
        }
    }

    if (parametrisation_.has("field.gridded")) {
        if (parametrisation_.has("user.grid")) {
            add(actions, "interpolate.grid2grid");
        }
    }

    if (parametrisation_.has("user.area")) {
        add(actions, "crop.area");
    }

    if (parametrisation_.has("user.bitmap")) {
        add(actions, "filter.bitmap");
    }

    if (parametrisation_.has("user.frame")) {
        add(actions, "filter.frame");
    }
}


static MIRLogicBuilder<MARSLogic> mars("mars");
