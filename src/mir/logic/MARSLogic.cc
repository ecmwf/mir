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

    bool autoresol = false;
    long intermediate_gaussian = 0;

    parametrisation_.get("autoresol", autoresol);
    parametrisation_.get("intermediate_gaussian", intermediate_gaussian);

    if (parametrisation_.has("field.spherical")) {
        if (parametrisation_.has("user.truncation")) {
            plan.add("transform.sh2sh");
        }
    }

    if (parametrisation_.has("field.spherical")) {
        if (parametrisation_.has("user.grid")) {
#if 0
            plan.add("transform.sh2regular-ll");
#else
            // For now, thar's what we do
            // runtime.set("reduced", 48L);

            if (autoresol) {
                plan.add("transform.sh2sh", "truncation", new AutoResol(parametrisation_));
            }

            if(intermediate_gaussian) {
                plan.add("transform.sh2reduced-gg", "reduced", intermediate_gaussian);
            }
            else
            {
            plan.add("transform.sh2reduced-gg", "reduced", new AutoReduced(parametrisation_));
        }
            plan.add("interpolate.grid2regular-ll");
#endif
        }
        if (parametrisation_.has("user.reduced")) {
            if (autoresol) {
                plan.add("transform.sh2sh", "truncation", new AutoResol(parametrisation_));
            }
            plan.add("transform.sh2reduced-gg");

        }
        if (parametrisation_.has("user.regular")) {
            if (autoresol) {
                plan.add("transform.sh2sh", "truncation", new AutoResol(parametrisation_));
            }
            plan.add("transform.sh2regular-gg");
        }
    }

    if (parametrisation_.has("field.gridded")) {
        if (parametrisation_.has("user.grid")) {
            plan.add("interpolate.grid2regular-ll");
        }
        if (parametrisation_.has("user.reduced")) {
            plan.add("interpolate.grid2reduced-gg");
        }
        if (parametrisation_.has("user.regular")) {
            plan.add("interpolate.grid2regular-gg");
        }
    }

    if (parametrisation_.has("user.area")) {
        plan.add("crop.area");
    }

    if (parametrisation_.has("user.bitmap")) {
        plan.add("filter.bitmap");
    }

    if (parametrisation_.has("user.frame")) {
        plan.add("filter.frame");
    }
}


// register MARS-specialized logic
namespace {
static MIRLogicBuilder<MARSLogic> mars("mars");
}


}  // namespace logic
}  // namespace mir

