// File MIRInput.cc
// Baudouin Raoult - (c) ECMWF Apr 15

#include "soyuz/logic/ToolLogic.h"
#include "soyuz/param/MIRParametrisation.h"

#include "eckit/runtime/Context.h"


ToolLogic::ToolLogic(const MIRParametrisation &parametrisation):
    MIRLogic(parametrisation) {
}

ToolLogic::~ToolLogic() {
}


void ToolLogic::print(std::ostream &out) const {
    out << "ToolLogic[]";
}

void ToolLogic::prepare(std::vector<std::auto_ptr<Action> > &actions) const {
    // All the nasty logic goes there

    size_t argc = eckit::Context::instance().argc();
    for(size_t i = 0; i < argc; i++) {
        std::cout << eckit::Context::instance().argv(i) << std::endl;
    }

}

static MIRLogicBuilder<ToolLogic> tool("tool");
