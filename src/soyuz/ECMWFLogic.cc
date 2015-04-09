// File MIRInput.cc
// Baudouin Raoult - (c) ECMWF Apr 15

#include "ECMWFLogic.h"
#include "MIRParametrisation.h"

ECMWFLogic::ECMWFLogic(const MIRParametrisation &parametrisation):
    MIRLogic(parametrisation) {
}

ECMWFLogic::~ECMWFLogic() {
}

void ECMWFLogic::prepare(std::vector<std::auto_ptr<Action> > &actions) const {
    // All the nasty logic goes there

    std::string ignore;

    if (parametrisation_.get("user.truncation", ignore)) {
        if (parametrisation_.get("field.truncation", ignore)) {
            add(actions, "transform.sh2sh");
        }
    }

    add(actions, "interpolate.noop"); // For the sake of the example

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
