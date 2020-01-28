/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 *
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/style/ECMWFStyle.h"

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "eckit/exception/Exceptions.h"
#include "eckit/log/Log.h"

#include "mir/action/io/Copy.h"
#include "mir/action/io/Save.h"
#include "mir/action/plan/ActionPlan.h"
#include "mir/config/LibMir.h"
#include "mir/output/MIROutput.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/param/SameParametrisation.h"
#include "mir/repres/latlon/LatLon.h"
#include "mir/style/Resol.h"
#include "mir/util/DeprecatedFunctionality.h"


namespace mir {
namespace style {


namespace {

static MIRStyleBuilder<ECMWFStyle> __style("ecmwf");

struct DeprecatedStyle : ECMWFStyle, util::DeprecatedFunctionality {
    DeprecatedStyle(const param::MIRParametrisation& p) :
        ECMWFStyle(p),
        util::DeprecatedFunctionality("style 'dissemination' now known as 'ecmwf'") {}
};

static MIRStyleBuilder<DeprecatedStyle> __deprecated_style("dissemination");


static std::string target_gridded_from_parametrisation(const param::MIRParametrisation& user,
                                                       const param::MIRParametrisation& field, bool checkRotation) {
    std::unique_ptr<const param::MIRParametrisation> same(new param::SameParametrisation(user, field, true));

    bool filter = false;
    user.get("filter", filter);

    std::vector<double> rotation;
    const bool rotated = checkRotation && user.has("rotation") && !same->get("rotation", rotation);

    bool forced = filter || rotated;
    const std::string prefix(user.has("rotation") ? "rotated-" : "");

    if (user.has("grid")) {
        std::vector<double> grid;
        return !same->get("grid", grid) || forced || !repres::latlon::LatLon::samePoints(user, field)
                   ? prefix + "regular-ll"
                   : "";
    }

    if (user.has("reduced")) {
        long N;
        return !same->get("reduced", N) || forced ? prefix + "reduced-gg" : "";
    }

    if (user.has("regular")) {
        long N;
        return !same->get("regular", N) || forced ? prefix + "regular-gg" : "";
    }

    if (user.has("octahedral")) {
        long N;
        return !same->get("octahedral", N) || forced ? prefix + "octahedral-gg" : "";
    }

    if (user.has("pl")) {
        std::vector<long> pl;
        return !same->get("pl", pl) || forced ? prefix + "reduced-gg-pl-given" : "";
    }

    if (user.has("gridname")) {
        std::string gridname;
        return !same->get("gridname", gridname) || forced ? prefix + "namedgrid" : "";
    }

    if (user.has("griddef")) {
        if (user.has("rotation")) {
            throw eckit::UserError("ECMWFStyle: option 'rotation' is incompatible with 'griddef'");
        }
        return "griddef";
    }

    if (user.has("latitudes") || user.has("longitudes")) {
        if (user.has("latitudes") != user.has("longitudes")) {
            throw eckit::UserError("ECMWFStyle: options 'latitudes' and 'longitudes' have to be provided together");
        }
        if (user.has("rotation")) {
            throw eckit::UserError("ECMWFStyle: option 'rotation' is incompatible with 'latitudes' and 'longitudes'");
        }
        return "points";
    }

    if (user.has("area") || user.has("rotation")) {
        std::vector<double> grid;
        if (field.get("grid", grid) && !repres::latlon::LatLon::samePoints(user, field)) {
            return prefix + "regular-ll";
        }
    }

    eckit::Log::debug<LibMir>() << "ECMWFStyle: did not determine target from parametrisation" << std::endl;
    return "";
}


void add_formula(action::ActionPlan& plan, const param::MIRParametrisation& param,
                 const std::vector<std::string>&& whens) {
    std::string formula;
    for (auto& when : whens) {
        if (param.get("formula." + when, formula)) {
            std::string metadata;  // paramId for the results of formulas
            param.get("formula." + when + ".metadata", metadata);

            plan.add("calc.formula", "formula", formula, "formula.metadata", metadata);
            break;
        }
    }
}


}  // namespace


ECMWFStyle::ECMWFStyle(const param::MIRParametrisation& parametrisation) : MIRStyle(parametrisation) {}


ECMWFStyle::~ECMWFStyle() = default;


void ECMWFStyle::prologue(action::ActionPlan& plan) const {
    auto& user = parametrisation_.userParametrisation();

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

    if (user.has("statistics") || user.has("input-statistics")) {
        plan.add("filter.statistics", "which-statistics", "input");
    }

    add_formula(plan, user, {"prologue"});
}


void ECMWFStyle::sh2grid(action::ActionPlan& plan) const {
    auto& user  = parametrisation_.userParametrisation();
    auto& field = parametrisation_.fieldParametrisation();

    add_formula(plan, user, {"spectral", "raw"});

    Resol resol(parametrisation_, false);

    bool rotation = user.has("rotation");

    bool vod2uv = false;
    bool uv2uv  = false;
    user.get("vod2uv", vod2uv);
    user.get("uv2uv", uv2uv);

    long uv = 0;
    uv2uv   = uv2uv || (field.get("is_wind_component_uv", uv) && uv);

    // completed later
    const std::string transform   = "transform." + std::string(vod2uv ? "sh-vod-to-uv-" : "sh-scalar-to-");
    const std::string interpolate = "interpolate.grid2";
    const std::string target      = target_gridded_from_parametrisation(user, field, false);

    if (resol.resultIsSpectral()) {
        resol.prepare(plan);
    }

    if (!target.empty()) {
        if (resol.resultIsSpectral()) {

            plan.add(transform + target);
        }
        else {

            resol.prepare(plan);

            // if the intermediate grid is the same as the target grid, the interpolation to the
            // intermediate grid is not followed by an additional interpolation
            std::string gridname;
            if (rotation || !user.get("gridname", gridname) || gridname != resol.gridname()) {
                plan.add(interpolate + target);
            }
        }

        if (vod2uv || uv2uv) {
            ASSERT(vod2uv != uv2uv);

            if (uv2uv) {
                plan.add("filter.adjust-winds-scale-cos-latitude");
            }

            if (rotation) {
                plan.add("filter.adjust-winds-directions");
            }
        }
    }

    add_formula(plan, user, {"gridded"});
}


void ECMWFStyle::sh2sh(action::ActionPlan& plan) const {
    auto& user = parametrisation_.userParametrisation();

    Resol resol(parametrisation_, true);
    eckit::Log::debug<LibMir>() << "ECMWFStyle: resol=" << resol << std::endl;

    // the runtime parametrisation above is needed to satisfy this assertion
    ASSERT(resol.resultIsSpectral());
    resol.prepare(plan);

    add_formula(plan, user, {"spectral", "raw"});

    bool vod2uv = false;
    user.get("vod2uv", vod2uv);

    if (vod2uv) {
        plan.add("transform.sh-vod-to-UV");
    }
}


void ECMWFStyle::grid2grid(action::ActionPlan& plan) const {
    auto& user  = parametrisation_.userParametrisation();
    auto& field = parametrisation_.fieldParametrisation();

    bool rotation = user.has("rotation");

    bool vod2uv = false;
    bool uv2uv  = false;
    user.get("vod2uv", vod2uv);
    user.get("uv2uv", uv2uv);

    if (vod2uv) {
        eckit::Log::error() << "ECMWFStyle: option 'vod2uv' does not support gridded input" << std::endl;
        ASSERT(!vod2uv);
    }

    add_formula(plan, user, {"gridded", "raw"});

    // completed later
    const std::string interpolate = "interpolate.grid2";
    const std::string target      = target_gridded_from_parametrisation(user, field, rotation);

    if (!target.empty()) {
        plan.add(interpolate + target);

        if (vod2uv || uv2uv) {
            ASSERT(vod2uv != uv2uv);

            if (rotation) {
                plan.add("filter.adjust-winds-directions");
            }
        }
    }
}


void ECMWFStyle::epilogue(action::ActionPlan& plan) const {
    auto& user = parametrisation_.userParametrisation();

    bool vod2uv = false;
    bool uv2uv  = false;
    user.get("vod2uv", vod2uv);
    user.get("uv2uv", uv2uv);

    if (vod2uv || uv2uv) {
        ASSERT(vod2uv != uv2uv);

        bool u_only = false;
        user.get("u-only", u_only);

        bool v_only = false;
        user.get("v-only", v_only);

        if (u_only) {
            ASSERT(!v_only);
            plan.add("select.field", "which", long(0));
        }

        if (v_only) {
            ASSERT(!u_only);
            plan.add("select.field", "which", long(1));
        }
    }

    add_formula(plan, user, {"epilogue"});

    std::string metadata;
    if (user.get("metadata", metadata)) {
        plan.add("set.metadata", "metadata", metadata);
    }

    if (user.has("statistics") || user.has("output-statistics")) {
        plan.add("filter.statistics", "which-statistics", "output");
    }

    std::string epilogue;
    if (parametrisation_.get("epilogue", epilogue)) {
        plan.add(epilogue);
    }
}


void ECMWFStyle::print(std::ostream& out) const {
    out << "ECMWFStyle[]";
}


void ECMWFStyle::prepare(action::ActionPlan& plan, input::MIRInput& input, output::MIROutput& output) const {

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

    if (parametrisation_.userParametrisation().has("latitudes") ||
        parametrisation_.userParametrisation().has("longitudes")) {
        user_wants_gridded++;
    }

    ASSERT(user_wants_gridded <= 1);

    bool field_gridded  = parametrisation_.fieldParametrisation().has("gridded");
    bool field_spectral = parametrisation_.fieldParametrisation().has("spectral");

    ASSERT(field_gridded != field_spectral);


    if (field_spectral) {
        if (user_wants_gridded) {
            sh2grid(plan);
        }
        else {
            // "user wants spectral"
            sh2sh(plan);
        }
    }


    if (field_gridded) {
        grid2grid(plan);
    }


    if (field_gridded || user_wants_gridded) {

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

        if (parametrisation_.userParametrisation().has("unstructured")) {
            plan.add("filter.unstructured");
        }
    }


    epilogue(plan);


    output.prepare(parametrisation_, plan, input, output);


    if (!plan.ended()) {
        if (plan.empty()) {
            plan.add(new action::io::Copy(parametrisation_, output));
        }
        else {
            plan.add(new action::io::Save(parametrisation_, input, output));
        }
    }
    ASSERT(plan.ended());
}


}  // namespace style
}  // namespace mir
