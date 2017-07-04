/*
 * (C) Copyright 1996-2017 ECMWF.
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
#include "eckit/exception/Exceptions.h"
#include "eckit/filesystem/PathName.h"
#include "mir/action/plan/ActionPlan.h"
#include "mir/config/LibMir.h"
#include "mir/param/MIRParametrisation.h"


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
    std::string formula;

    ASSERT(field_gridded != field_spectral);


    if (field_spectral) {
        if (user_wants_gridded) {

            if (parametrisation_.get("user.formula.spectral", formula)) {
                std::string metadata;
                // paramId for the results of formulas
                parametrisation_.get("user.formula.spectral.metadata", metadata);

                plan.add("calc.formula", "formula", formula, "formula.metadata", metadata);
            }

            sh2grid(plan);

            if (parametrisation_.get("user.formula.gridded", formula)) {
                std::string metadata;
                // paramId for the results of formulas
                parametrisation_.get("user.formula.gridded.metadata", metadata);
                plan.add("calc.formula", "formula", formula, "formula.metadata", metadata);
            }

        } else {
            // "user wants spectral"
            sh2sh(plan);
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

    epilogue(plan);

}


bool ECMWFStyle::forcedPrepare(const param::MIRParametrisation& parametrisation) const {
    static const char *force[] = {
        "accuracy",
        "bitmap",
        "checkerboard",
        "edition",
        "formula",
        "frame",
        "packing",
        "pattern",
        "vod2uv",
        0
    };

    bool forced = false;
    for (size_t i = 0; force[i] && !forced; ++i) {
        forced = parametrisation.has(force[i]);
    }
    return forced;
}


void ECMWFStyle::grid2grid(action::ActionPlan& plan) const {

    bool vod2uv = false;
    parametrisation_.get("vod2uv", vod2uv);

    bool wind = false;
    parametrisation_.get("wind", wind);

    if (parametrisation_.has("user.pl") && parametrisation_.has("user.rotation")) {
        throw eckit::UserError("'user.pl' is incompatible with option 'rotation'.");
    }


    const std::string userGrid =
        parametrisation_.has("user.grid")?          "regular-ll" :
        parametrisation_.has("user.reduced") ?      "reduced-gg" :
        parametrisation_.has("user.regular") ?      "regular-gg" :
        parametrisation_.has("user.octahedral") ?   "octahedral-gg" :
        parametrisation_.has("user.pl") ?           "reduced-gg-pl-given" :
        parametrisation_.has("user.gridname") ?     "namedgrid" :
        parametrisation_.has("user.griddef") ?      "griddef" :
        "";

    if (userGrid.length()) {
        if (parametrisation_.has("user.rotation")) {
            plan.add("interpolate.grid2rotated-" + userGrid);
            if (wind || vod2uv) {
                plan.add("filter.adjust-winds-directions");
                selectWindComponents(plan);
            }
        } else {
            plan.add("interpolate.grid2" + userGrid);
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


void ECMWFStyle::sh2sh(action::ActionPlan& plan) const {

    if (parametrisation_.has("user.truncation")) {
        plan.add("transform.sh-truncate");
    }

    std::string formula;
    if (parametrisation_.get("user.formula.spectral", formula)) {
        std::string metadata;
        // paramId for the results of formulas
        parametrisation_.get("user.formula.spectral.metadata", metadata);

        plan.add("calc.formula", "formula", formula, "formula.metadata", metadata);
    }

    bool vod2uv = false;
    parametrisation_.get("vod2uv", vod2uv);

    if (vod2uv) {
        plan.add("transform.sh-vod-to-UV");
    }

    selectWindComponents(plan);
}


void ECMWFStyle::epilogue(action::ActionPlan& plan) const {

    bool globalise = false;
    parametrisation_.get("user.globalise", globalise);

    if (globalise) {
        plan.add("filter.globalise");
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

    std::string formula;
    if (parametrisation_.get("user.formula.epilogue", formula)) {
        std::string metadata;
        // paramId for the results of formulas
        parametrisation_.get("user.formula.epilogue.metadata", metadata);
        plan.add("calc.formula", "formula", formula, "formula.metadata", metadata);
    }

    std::string metadata;
    if (parametrisation_.get("user.metadata", metadata)) {
        plan.add("set.metadata", "metadata", metadata);
    }

    std::string epilogue;
    if (parametrisation_.get("epilogue", epilogue)) {
        plan.add(epilogue);
    }

}


bool ECMWFStyle::isWindComponent() const {
    long id = 0;
    parametrisation_.get("paramId", id);

    const long id_u = 131;
    const long id_v = 132;

    return (id == id_u || id == id_v);
}


bool ECMWFStyle::selectWindComponents(action::ActionPlan& plan) const {
    bool u_only = false;
    if (parametrisation_.get("u-only", u_only) && u_only) {
        plan.add("select.field", "which", long(0));
    }
    bool v_only = false;
    if (parametrisation_.get("v-only", v_only) && v_only) {
        ASSERT(!u_only);
        plan.add("select.field", "which", long(1));
    }
    return (u_only || v_only);
}


}  // namespace style
}  // namespace mir

