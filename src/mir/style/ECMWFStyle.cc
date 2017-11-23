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
#include "mir/action/plan/ActionPlan.h"
#include "mir/api/MIRJob.h"
#include "mir/config/LibMir.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/style/IntermediateGrid.h"
#include "mir/style/Resol.h"
#include "mir/style/resol/Truncation.h"
#include "mir/util/DeprecatedFunctionality.h"


namespace mir {
namespace style {


namespace {
static MIRStyleBuilder<ECMWFStyle> __style("ecmwf");

struct DeprecatedStyle : ECMWFStyle, util::DeprecatedFunctionality {
    DeprecatedStyle(const param::MIRParametrisation& p) : ECMWFStyle(p), util::DeprecatedFunctionality("style 'dissemination' now known as 'ecmwf'") {}
};
static MIRStyleBuilder<DeprecatedStyle> __deprecated_style("dissemination");
}


ECMWFStyle::ECMWFStyle(const param::MIRParametrisation& parametrisation):
    MIRStyle(parametrisation) {
}


ECMWFStyle::~ECMWFStyle() {
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
    if (parametrisation_.userParametrisation().get("formula.prologue", formula)) {
        std::string metadata;
        // paramId for the results of formulas
        parametrisation_.userParametrisation().get("formula.prologue.metadata", metadata);
        plan.add("calc.formula", "formula", formula, "formula.metadata", metadata);
    }
}


void ECMWFStyle::sh2grid(action::ActionPlan& plan) const {

    std::string formula;
    if (parametrisation_.userParametrisation().get("formula.spectral", formula) ||
            parametrisation_.userParametrisation().get("formula.raw", formula)
       ) {
        std::string metadata;
        // paramId for the results of formulas
        parametrisation_.userParametrisation().get("formula.spectral.metadata", metadata);

        plan.add("calc.formula", "formula", formula, "formula.metadata", metadata);
    }


    // FIXME make a decision on resol/truncation!
    std::string resol = "automatic-resolution";
    parametrisation_.get("resol", resol);
    eckit::ScopedPtr<Resol> resolution(ResolFactory::build(resol, parametrisation_));

    long T = 0;
    if (parametrisation_.userParametrisation().get("truncation", T)) {
        // this is overriding for the moment until a decision is taken
        resolution.reset(new resol::Truncation(T, parametrisation_));
    }
    ASSERT(resolution);

    if (resolution->resultIsSpectral()) {
        resolution->prepare(plan);
    }

    bool vod2uv = false;
    parametrisation_.userParametrisation().get("vod2uv", vod2uv);
    std::string transform = vod2uv ? "sh-vod-to-uv-" : "sh-scalar-to-"; // completed later

    if (parametrisation_.userParametrisation().has("grid")) {

        if (resolution->resultIsSpectral()) {
            plan.add("transform." + transform + "regular-ll");
        } else {
            resolution->prepare(plan);
            plan.add("interpolate.grid2regular-ll");
        }

        if (parametrisation_.userParametrisation().has("rotation")) {
            plan.add("interpolate.grid2rotated-regular-ll");

            bool wind = false;
            parametrisation_.userParametrisation().get("wind", wind);

            if (wind || vod2uv) {
                plan.add("filter.adjust-winds-directions");
                selectWindComponents(plan);
            }
        }
    }

    if (parametrisation_.userParametrisation().has("reduced")) {
        plan.add("transform." + transform + "reduced-gg");
    }

    if (parametrisation_.userParametrisation().has("regular")) {
        plan.add("transform." + transform + "regular-gg");
    }

    if (parametrisation_.userParametrisation().has("octahedral")) {
        plan.add("transform." + transform + "octahedral-gg");
    }

    if (parametrisation_.userParametrisation().has("pl")) {
        plan.add("transform." + transform + "reduced-gg-pl-given");
    }

    if (parametrisation_.userParametrisation().has("gridname")) {
        std::string gridname;
        ASSERT(parametrisation_.get("gridname", gridname));
        plan.add("transform." + transform + "namedgrid");
    }

    if (parametrisation_.userParametrisation().has("griddef")) {
        std::string griddef;
        ASSERT(parametrisation_.get("griddef", griddef));
        // TODO: this is temporary
        plan.add("transform." + transform + "octahedral-gg", "octahedral", 64L);
        plan.add("interpolate.grid2griddef");
    }

    if (isWindComponent()) {
        plan.add("filter.adjust-winds-scale-cos-latitude");
    }

    if (!parametrisation_.userParametrisation().has("rotation")) {
        selectWindComponents(plan);
    }

    if (parametrisation_.userParametrisation().get("formula.gridded", formula)) {
        std::string metadata;
        // paramId for the results of formulas
        parametrisation_.userParametrisation().get("formula.gridded.metadata", metadata);
        plan.add("calc.formula", "formula", formula, "formula.metadata", metadata);
    }
}


void ECMWFStyle::sh2sh(action::ActionPlan& plan) const {

    // FIXME make a decision on resol/truncation!
    std::string resol = "automatic-resolution";
    parametrisation_.get("resol", resol);
    eckit::ScopedPtr<Resol> resolution(ResolFactory::build(resol, parametrisation_));

    long T = 0;
    if (parametrisation_.userParametrisation().get("truncation", T)) {
        // this is overriding for the moment until a decision is taken
        resolution.reset(new resol::Truncation(T, parametrisation_));
    }
    ASSERT(resolution);

    if (resolution->resultIsSpectral()) {
        resolution->prepare(plan);
    }

    std::string formula;
    if (parametrisation_.userParametrisation().get("formula.spectral", formula)) {
        std::string metadata;
        // paramId for the results of formulas
        parametrisation_.userParametrisation().get("formula.spectral.metadata", metadata);

        plan.add("calc.formula", "formula", formula, "formula.metadata", metadata);
    }

    bool vod2uv = false;
    parametrisation_.userParametrisation().get("vod2uv", vod2uv);

    if (vod2uv) {
        plan.add("transform.sh-vod-to-UV");
    }

    selectWindComponents(plan);
}


void ECMWFStyle::grid2grid(action::ActionPlan& plan) const {

    bool vod2uv = false;
    parametrisation_.userParametrisation().get("vod2uv", vod2uv);

    bool wind = false;
    parametrisation_.userParametrisation().get("wind", wind);

    if (parametrisation_.userParametrisation().has("pl") && parametrisation_.userParametrisation().has("rotation")) {
        throw eckit::UserError("'user.pl' is incompatible with option 'rotation'.");
    }


    const std::string userGrid =
        parametrisation_.userParametrisation().has("grid") ?         "regular-ll" :
        parametrisation_.userParametrisation().has("reduced") ?      "reduced-gg" :
        parametrisation_.userParametrisation().has("regular") ?      "regular-gg" :
        parametrisation_.userParametrisation().has("octahedral") ?   "octahedral-gg" :
        parametrisation_.userParametrisation().has("pl") ?           "reduced-gg-pl-given" :
        parametrisation_.userParametrisation().has("gridname") ?     "namedgrid" :
        parametrisation_.userParametrisation().has("griddef") ?      "griddef" :
        parametrisation_.userParametrisation().has("points") ?       "points" :
        "";

    std::string formula;
    if (parametrisation_.userParametrisation().get("formula.gridded", formula) ||
            parametrisation_.userParametrisation().get("formula.raw", formula)) {
        std::string metadata;
        // paramId for the results of formulas
        parametrisation_.userParametrisation().get("formula.gridded.metadata", metadata);
        plan.add("calc.formula", "formula", formula, "formula.metadata", metadata);
    }

    if (userGrid.length()) {
       if (parametrisation_.userParametrisation().has("rotation")) {
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


void ECMWFStyle::epilogue(action::ActionPlan& plan) const {

    bool globalise = false;
    parametrisation_.userParametrisation().get("globalise", globalise);

    if (globalise) {
        plan.add("filter.globalise");
    }

    if (parametrisation_.userParametrisation().has("area")) {
        plan.add("crop.area");
    }

    if (parametrisation_.userParametrisation().has("bitmap")) {
        plan.add("filter.bitmap");
    }

    if (parametrisation_.userParametrisation().has("frame")) {
        plan.add("filter.frame");
    }

    std::string formula;
    if (parametrisation_.userParametrisation().get("formula.epilogue", formula)) {
        std::string metadata;
        // paramId for the results of formulas
        parametrisation_.userParametrisation().get("formula.epilogue.metadata", metadata);
        plan.add("calc.formula", "formula", formula, "formula.metadata", metadata);
    }

    std::string metadata;
    if (parametrisation_.userParametrisation().get("metadata", metadata)) {
        plan.add("set.metadata", "metadata", metadata);
    }

    std::string epilogue;
    if (parametrisation_.get("epilogue", epilogue)) {
        plan.add(epilogue);
    }
}


void ECMWFStyle::print(std::ostream& out) const {
    out << "ECMWFStyle[]";
}


bool ECMWFStyle::isWindComponent() const {
    long id = 0;
    parametrisation_.get("paramId", id);

    if (id == 0) {
        return false;
    }

    const eckit::Configuration& config = LibMir::instance().configuration();
    const long id_u = config.getLong("parameter-id-u", 131);
    const long id_v = config.getLong("parameter-id-v", 132);

    return (id == id_u || id == id_v);
}


bool ECMWFStyle::selectWindComponents(action::ActionPlan& plan) const {
    bool u_only = false;
    if (parametrisation_.userParametrisation().get("u-only", u_only) && u_only) {
        plan.add("select.field", "which", long(0));
    }
    bool v_only = false;
    if (parametrisation_.userParametrisation().get("v-only", v_only) && v_only) {
        ASSERT(!u_only);
        plan.add("select.field", "which", long(1));
    }
    return (u_only || v_only);
}


void ECMWFStyle::prepare(action::ActionPlan& plan) const {

    // All the nasty logic goes there
    prologue(plan);

    size_t user_wants_gridded = 0;

    if (parametrisation_.userParametrisation().has("grid")) {
        user_wants_gridded++;
    }

    if (parametrisation_.userParametrisation().has("reduced")) {
        user_wants_gridded++;
    }

    if (parametrisation_.userParametrisation().has("regular")) {
        user_wants_gridded++;
    }

    if (parametrisation_.userParametrisation().has("octahedral")) {
        user_wants_gridded++;
    }

    if (parametrisation_.userParametrisation().has("pl")) {
        user_wants_gridded++;
    }

    if (parametrisation_.userParametrisation().has("gridname")) {
        user_wants_gridded++;
    }

    if (parametrisation_.userParametrisation().has("griddef")) {
        user_wants_gridded++;
    }

    if (parametrisation_.userParametrisation().has("points")) {
        user_wants_gridded++;
    }

    ASSERT(user_wants_gridded <= 1);

    bool field_gridded  = parametrisation_.fieldParametrisation().has("gridded");
    bool field_spectral = parametrisation_.fieldParametrisation().has("spectral");

    ASSERT(field_gridded != field_spectral);


    if (field_spectral) {
        if (user_wants_gridded) {
            sh2grid(plan);
        } else {
            // "user wants spectral"
            sh2sh(plan);
        }
    }


    if (field_gridded) {

        std::string formula;
        if (parametrisation_.userParametrisation().get("formula.gridded", formula)) {
            std::string metadata;
            // paramId for the results of formulas
            parametrisation_.userParametrisation().get("formula.gridded.metadata", metadata);
            plan.add("calc.formula", "formula", formula, "formula.metadata", metadata);
        }
        grid2grid(plan);
    }

    epilogue(plan);
}


bool ECMWFStyle::postProcessingRequested(const api::MIRJob& job) const {
    static const char *force[] = {
        "accuracy",
        "bitmap",
        "checkerboard",
        "griddef",
        "points",
        "edition",
        "formula",
        "frame",
        "packing",
        "pattern",
        "vod2uv",
        0
    };

    for (size_t i = 0; force[i]; ++i) {
        if (job.has(force[i])) {
            return true;
        }
    }

    return false;
}


}  // namespace style
}  // namespace mir

