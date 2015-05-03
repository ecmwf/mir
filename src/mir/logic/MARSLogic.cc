/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Pedro Maciel
/// @date Apr 2015


#include <iostream>

#include "mir/logic/MARSLogic.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/action/ActionPlan.h"
#include "mir/logic/AutoResol.h"
#include "mir/logic/AutoReduced.h"
#include "mir/param/MIRConfiguration.h"
#include "mir/param/MIRCombinedParametrisation.h"
#include "mir/param/MIRDefaults.h"
#include "eckit/exception/Exceptions.h"


namespace mir {
namespace logic {


MARSLogic::MARSLogic(const param::MIRParametrisation &parametrisation):
    MIRLogic(parametrisation) {

}


MARSLogic::~MARSLogic() {
}


void MARSLogic::print(std::ostream &out) const {
    out << "MARSLogic[]";
}


void MARSLogic::prepare(action::ActionPlan &plan) const {
    // All the nasty logic goes there

    // Accroding to c++11, this should be thread safe (assuming contructors are thread safe as well)
    const param::MIRConfiguration& configuration = param::MIRConfiguration::instance();

    long paramId = 0;
    ASSERT(parametrisation_.get("paramId", paramId));

    const param::SimpleParametrisation* param = configuration.lookup(paramId);
    if(param) {
        eckit::Log::info() << "Parametrisation for paramId " << paramId << " is " << *param << std::endl;
        param::MIRCombinedParametrisation combined(parametrisation_, *param, param::MIRDefaults::instance());
        prepare(plan, combined);
    } else {
        // Use default parametrisation
        prepare(plan, parametrisation_);
    }
}

void MARSLogic::prepare(action::ActionPlan &plan, const param::MIRParametrisation &parametrisation) const {

    bool autoresol = false;
    bool vod2uv = false;

    long intermediate_gaussian = 0;
    parametrisation.get("autoresol", autoresol);
    parametrisation.get("vod2uv", vod2uv);

    parametrisation.get("intermediate_gaussian", intermediate_gaussian);

    if (parametrisation.has("field.spherical")) {
        if (parametrisation.has("user.truncation")) {
            plan.add("transform.sh2sh");
        }
    }

    if(vod2uv) {
        plan.add("transform.vod2uv");
    }

    if (parametrisation.has("field.spherical")) {
        if (parametrisation.has("user.grid")) {
#if 0
            plan.add("transform.sh2regular-ll");
#else
            // For now, thar's what we do
            // runtime.set("reduced", 48L);

            if (autoresol) {
                plan.add("transform.sh2sh", "truncation", new AutoResol(parametrisation));
            }

            if(intermediate_gaussian) {
                plan.add("transform.sh2reduced-gg", "reduced", intermediate_gaussian);
            } else {
                plan.add("transform.sh2reduced-gg", "reduced", new AutoReduced(parametrisation));
            }
            plan.add("interpolate.grid2regular-ll");
#endif
        }
        if (parametrisation.has("user.reduced")) {
            if (autoresol) {
                plan.add("transform.sh2sh", "truncation", new AutoResol(parametrisation));
            }
            plan.add("transform.sh2reduced-gg");

        }
        if (parametrisation.has("user.regular")) {
            if (autoresol) {
                plan.add("transform.sh2sh", "truncation", new AutoResol(parametrisation));
            }
            plan.add("transform.sh2regular-gg");
        }
        if (parametrisation.has("user.octahedral")) {
            if (autoresol) {
                plan.add("transform.sh2sh", "truncation", new AutoResol(parametrisation));
            }
            plan.add("transform.sh2octahedral-gg");
        }
    }

    if (parametrisation.has("field.gridded")) {
        if (parametrisation.has("user.grid")) {
            plan.add("interpolate.grid2regular-ll");
        }
        if (parametrisation.has("user.reduced")) {
            plan.add("interpolate.grid2reduced-gg");
        }
        if (parametrisation.has("user.regular")) {
            plan.add("interpolate.grid2regular-gg");
        }
    }

    if (parametrisation.has("user.area")) {
        plan.add("crop.area");
    }

    if (parametrisation.has("user.bitmap")) {
        plan.add("filter.bitmap");
    }

    if (parametrisation.has("user.frame")) {
        plan.add("filter.frame");
    }
}


// register MARS-specialized logic
namespace {
static MIRLogicBuilder<MARSLogic> mars("mars");
}


}  // namespace logic
}  // namespace mir

