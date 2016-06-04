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

#include "mir/style/ECMWFStyle.h"

#include <iostream>

#include "mir/param/MIRParametrisation.h"
#include "mir/action/plan/ActionPlan.h"
#include "eckit/exception/Exceptions.h"


namespace mir {
namespace style {


ECMWFStyle::ECMWFStyle(const param::MIRParametrisation &parametrisation):
    MIRStyle(parametrisation) {

}


ECMWFStyle::~ECMWFStyle() {
}


void ECMWFStyle::prepare(action::ActionPlan &plan) const {
    // All the nasty style goes there

    prologue(plan);

    size_t user_wants_gridded = 0;

    if (parametrisation_.has("user.grid")) {
        user_wants_gridded++;
    }

    if (parametrisation_.has("user.reduced")) {
        user_wants_gridded++;
    }

    if (parametrisation_.has("user.regular")) {
        user_wants_gridded++;
    }

    if (parametrisation_.has("user.octahedral")) {
        user_wants_gridded++;
    }

    if (parametrisation_.has("user.pl")) {
        user_wants_gridded++;
    }

    if (parametrisation_.has("user.gridname")) {
        user_wants_gridded++;
    }

    if (parametrisation_.has("user.griddef")) {
        user_wants_gridded++;
    }

    ASSERT(user_wants_gridded <= 1);

    bool field_gridded  = parametrisation_.has("field.gridded");
    bool field_spectral = parametrisation_.has("field.spectral");

    ASSERT(field_gridded != field_spectral);

    if (field_spectral) {
        sh2sh(plan);
        if (user_wants_gridded) {
            sh2grid(plan);
        }
    }

    if (field_gridded) {
        grid2grid(plan);
    }

    epilogue(plan);

    // std::cout << plan << std::endl;
}

void ECMWFStyle::grid2grid(action::ActionPlan& plan) const {

    bool vod2uv = false;
    parametrisation_.get("vod2uv", vod2uv);

    bool wind = false;
    parametrisation_.get("wind", wind);

    if (parametrisation_.has("user.grid")) {
        if (parametrisation_.has("user.rotation")) {
            plan.add("interpolate.grid2rotated-regular-ll");
            if (wind || vod2uv) {
                plan.add("filter.adjust-winds");
            }
        } else {
            plan.add("interpolate.grid2regular-ll");
        }
    }

    if (parametrisation_.has("user.reduced")) {
        if (parametrisation_.has("user.rotation")) {
            plan.add("interpolate.grid2rotated-reduced-gg");
            if (wind || vod2uv) {
                plan.add("filter.adjust-winds");
            }
        } else {
            plan.add("interpolate.grid2reduced-gg");
        }
    }

    if (parametrisation_.has("user.regular")) {
        if (parametrisation_.has("user.rotation")) {
            plan.add("interpolate.grid2rotated-regular-gg");
            if (wind || vod2uv) {
                plan.add("filter.adjust-winds");
            }
        } else {
            plan.add("interpolate.grid2regular-gg");
        }
    }

    if (parametrisation_.has("user.octahedral")) {
        if (parametrisation_.has("user.rotation")) {
            plan.add("interpolate.grid2rotated-octahedral-gg");
            if (wind || vod2uv) {
                plan.add("filter.adjust-winds");
            }
        } else {
            plan.add("interpolate.grid2octahedral-gg");
        }
    }

    if (parametrisation_.has("user.pl")) {
        ASSERT(!parametrisation_.has("user.rotation"));
        plan.add("interpolate.grid2reduced-gg-pl-given");
    }

    if (parametrisation_.has("user.gridname")) {
        std::string gridname;
        ASSERT (parametrisation_.get("gridname", gridname));

        if (parametrisation_.has("user.rotation")) {
            plan.add("interpolate.grid2rotated-namedgrid");
            if (wind || vod2uv) {
                plan.add("filter.adjust-winds");
            }
        } else {
            plan.add("interpolate.grid2namedgrid");
        }
    }

    if (parametrisation_.has("user.griddef")) {
        std::string griddef;
        ASSERT (parametrisation_.get("griddef", griddef));

        if (parametrisation_.has("user.rotation")) {
            plan.add("interpolate.grid2rotated-griddef");
            if (wind || vod2uv) {
                plan.add("filter.adjust-winds");
            }
        } else {
            plan.add("interpolate.grid2griddef");
        }
    }

}

void ECMWFStyle::prologue(action::ActionPlan& plan) const {

    if (parametrisation_.has("checkerboard")) {
        plan.add("misc.checkerboard");
    }

    if (parametrisation_.has("pattern")) {
        plan.add("misc.pattern");
    }

    if (parametrisation_.has("add.fields")) {
        plan.add("add.fields");
    }

}

void ECMWFStyle::epilogue(action::ActionPlan& plan) const {

    // Could be in the prologue
    if (parametrisation_.has("multiply.scalar")) {
        plan.add("multiply.scalar");
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


}  // namespace style
}  // namespace mir

