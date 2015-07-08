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


    bool autoresol = false;
    bool vod2uv = false;

    long intermediate_gaussian = 0;
    parametrisation_.get("autoresol", autoresol);
    parametrisation_.get("vod2uv", vod2uv);
    parametrisation_.get("intermediate_gaussian", intermediate_gaussian);

    bool user_grid = parametrisation_.has("user.grid");
    bool user_reduced = parametrisation_.has("user.reduced");
    bool user_regular = parametrisation_.has("user.regular");
    bool user_octahedral = parametrisation_.has("user.octahedral");

    if (user_grid) {
        ASSERT(!user_reduced);
        ASSERT(!user_regular);
        ASSERT(!user_octahedral);
    }

    if (user_reduced) {
        ASSERT(!user_grid);
        ASSERT(!user_regular);
        ASSERT(!user_octahedral);
    }

    if (user_regular) {
        ASSERT(!user_grid);
        ASSERT(!user_reduced);
        ASSERT(!user_octahedral);
    }

    if (user_octahedral) {
        ASSERT(!user_grid);
        ASSERT(!user_reduced);
        ASSERT(!user_regular);
    }
    if (parametrisation_.has("field.spectral")) {
        if (parametrisation_.has("user.truncation")) {
            plan.add("transform.sh2sh");
        }


        if (vod2uv) {
            plan.add("transform.vod2uv");
        }



        if (user_grid) {

            if (autoresol) {
                plan.add("transform.sh2sh", "truncation", new AutoResol(parametrisation_));
            }

            if (intermediate_gaussian) {
                plan.add("transform.sh2reduced-gg", "reduced", intermediate_gaussian);
                plan.add("interpolate.grid2regular-ll");
            } else {
                plan.add("transform.sh2regular-ll");
            }

            if (parametrisation_.has("user.rotation")) {
                plan.add("interpolate.grid2rotated-regular-ll");
            }

        }

        if (user_reduced) {
            if (autoresol) {
                plan.add("transform.sh2sh", "truncation", new AutoResol(parametrisation_));
            }
            plan.add("transform.sh2reduced-gg");

        }

        if (user_regular) {
            if (autoresol) {
                plan.add("transform.sh2sh", "truncation", new AutoResol(parametrisation_));
            }
            plan.add("transform.sh2regular-gg");
        }

        if (user_octahedral) {
            if (autoresol) {
                plan.add("transform.sh2sh", "truncation", new AutoResol(parametrisation_));
            }
            plan.add("transform.sh2octahedral-gg");
        }
    }

    else if (parametrisation_.has("field.gridded")) {

        if (user_grid) {
            if (parametrisation_.has("user.rotation")) {
                plan.add("interpolate.grid2rotated-regular-ll");
            } else {
                plan.add("interpolate.grid2regular-ll");
            }
        }

        if (user_reduced) {
            if (parametrisation_.has("user.rotation")) {
                plan.add("interpolate.grid2rotated-reduced-gg");
            } else {
                plan.add("interpolate.grid2reduced-gg");
            }
        }

        if (user_regular) {
            if (parametrisation_.has("user.rotation")) {
                plan.add("interpolate.grid2rotated-regular-gg");
            } else {
                plan.add("interpolate.grid2regular-gg");
            }
        }

        if (user_octahedral) {
            if (parametrisation_.has("user.rotation")) {
                plan.add("interpolate.grid2rotated-octahedral-gg");
            } else {
                plan.add("interpolate.grid2octahedral-gg");
            }
        }
    } else {
        throw eckit::SeriousBug("Input field in neither spectral nor gridded");
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

