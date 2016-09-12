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
#include "eckit/filesystem/PathName.h"
#include "eckit/exception/Exceptions.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/action/plan/ActionPlan.h"


namespace mir {
namespace style {


ECMWFStyle::ECMWFStyle(const param::MIRParametrisation &parametrisation):
    MIRStyle(parametrisation) {

}


ECMWFStyle::~ECMWFStyle() {
}

void ECMWFStyle::selectWindComponents(action::ActionPlan& plan) const {
    bool u_only = false;
    if (parametrisation_.get("u-only", u_only) && u_only) {
        plan.add("select.field", "which", long(0));
    }
    bool v_only = false;
    if (parametrisation_.get("v-only", v_only) && v_only) {
        plan.add("select.field", "which", long(1));
    }
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
    std::string formula;

    ASSERT(field_gridded != field_spectral);


    if (field_spectral) {
        sh2sh(plan);

        if (parametrisation_.get("user.formula.spectral", formula)) {
            std::string metadata;
            // paramId for the results of formulas
            parametrisation_.get("user.formula.spectral.metadata", metadata);

            plan.add("calc.formula", "formula", formula, "formula.metadata", metadata);
        }

        if (user_wants_gridded) {
            sh2grid(plan);

            if (parametrisation_.get("user.formula.gridded", formula)) {
                std::string metadata;
                // paramId for the results of formulas
                parametrisation_.get("user.formula.gridded.metadata", metadata);
                plan.add("calc.formula", "formula", formula, "formula.metadata", metadata);
            }
        }
        else {
            selectWindComponents(plan);
        }
    }


    if (field_gridded) {

        if (parametrisation_.get("user.formula.gridded", formula)) {
            std::string metadata;
            // paramId for the results of formulas
            parametrisation_.get("user.formula.gridded.metadata", metadata);
            plan.add("calc.formula", "formula", formula, "formula.metadata", metadata);
        }
        grid2grid(plan);
    }


#if 0
    std::string x;
    if (parametrisation_.has("compare")) {
        parametrisation_.get("compare", x);
        eckit::PathName k(x);
        std::cout << parametrisation_ << std::endl;
        std::cout << "x = \"" << x << "\"" << std::endl;
        std::cout << "k = \"" << k << "\"" << std::endl;
    }
    else {
        std::cout << parametrisation_ << std::endl;
    }
#endif


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
                selectWindComponents(plan);
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
                selectWindComponents(plan);
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
                selectWindComponents(plan);
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
                selectWindComponents(plan);
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
                selectWindComponents(plan);
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
                selectWindComponents(plan);
            }
        } else {
            plan.add("interpolate.grid2griddef");
        }
    }

}

void ECMWFStyle::prologue(action::ActionPlan& plan) const {

    std::string prologue;
    if (parametrisation_.get("prologue", prologue)) {
        plan.add(prologue);
    }

    if (parametrisation_.has("checkerboard")) {
        plan.add("misc.checkerboard");
    }

    if (parametrisation_.has("pattern")) {
        plan.add("misc.pattern");
    }

    std::string formula;
    if (parametrisation_.get("user.formula.prologue", formula)) {
        std::string metadata;
        // paramId for the results of formulas
        parametrisation_.get("user.formula.prologue.metadata", metadata);
        plan.add("calc.formula", "formula", formula, "formula.metadata", metadata);
    }
}

void ECMWFStyle::epilogue(action::ActionPlan& plan) const {

    if (parametrisation_.has("user.area")) {
        plan.add("crop.area");
    }

    if (parametrisation_.has("user.bitmap")) {
        plan.add("filter.bitmap");
    }

    if (parametrisation_.has("user.frame")) {
        plan.add("filter.frame");
    }

    std::string formula;
    if (parametrisation_.get("user.formula.epilogue", formula)) {
        std::string metadata;
        // paramId for the results of formulas
        parametrisation_.get("user.formula.epilogue.metadata", metadata);
        plan.add("calc.formula", "formula", formula, "formula.metadata", metadata);
    }

    std::string epilogue;
    if (parametrisation_.get("epilogue", epilogue)) {
        plan.add(epilogue);
    }

}


}  // namespace style
}  // namespace mir

