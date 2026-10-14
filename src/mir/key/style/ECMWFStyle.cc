// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/key/style/ECMWFStyle.h"

#include <algorithm>
#include <iterator>
#include <memory>
#include <ostream>
#include <string>
#include <vector>

#include "eckit/filesystem/PathName.h"
#include "eckit/parser/YAMLParser.h"
#include "eckit/utils/StringTools.h"

#include "mir/action/plan/ActionPlan.h"
#include "mir/config/LibMir.h"
#include "mir/key/Area.h"
#include "mir/key/grid/Grid.h"
#include "mir/key/intgrid/Intgrid.h"
#include "mir/key/truncation/Truncation.h"
#include "mir/output/MIROutput.h"
#include "mir/param/CombinedParametrisation.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/param/RuntimeParametrisation.h"
#include "mir/param/SameParametrisation.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/repres/latlon/LatLon.h"
#include "mir/util/BoundingBox.h"
#include "mir/util/DeprecatedFunctionality.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Types.h"
#include "mir/util/ValueMap.h"


namespace mir::key::style {


struct DeprecatedStyle : ECMWFStyle, util::DeprecatedFunctionality {
    explicit DeprecatedStyle(const param::MIRParametrisation& p) :
        ECMWFStyle(p), util::DeprecatedFunctionality("style 'dissemination' now known as 'ecmwf'") {}
};


static const MIRStyleBuilder<ECMWFStyle> __style("ecmwf");
static const MIRStyleBuilder<DeprecatedStyle> __deprecated_style("dissemination");


static bool option(const param::MIRParametrisation& param, const std::string& key, bool dfault) {
    bool value = dfault;
    param.get(key, value);
    return value;
}


static bool same_points(const param::MIRParametrisation& user, const param::MIRParametrisation& field) {
    std::unique_ptr<const param::MIRParametrisation> same(new param::SameParametrisation(user, field, true));

    std::vector<double> rotation;
    if (user.has("rotation") && !same->get("rotation", rotation)) {
        return false;
    }

    std::vector<double> grid;
    if (user.has("grid") && !same->get("grid", grid)) {
        return false;
    }

    util::BoundingBox bboxUser;
    if (Area::get(user, bboxUser)) {
        util::Increments inc(field);
        size_t ni = 0;
        size_t nj = 0;

        repres::latlon::LatLon::correctBoundingBox(bboxUser, ni, nj, inc, {bboxUser.south(), bboxUser.west()});

        util::BoundingBox bboxField(field);
        repres::latlon::LatLon::correctBoundingBox(bboxField, ni, nj, inc, {bboxField.south(), bboxField.west()});

        PointLatLon ref{bboxField.south(), bboxField.west()};

        for (const auto& lat : {bboxUser.south(), bboxUser.north()}) {
            for (const auto& lon : {bboxUser.east(), bboxUser.west()}) {
                if (inc.isShifted({ref.lat() - lat, ref.lon() - lon})) {
                    return false;
                }
            }
        }
    }

    return true;
}


static std::string target_gridded_from_parametrisation(const param::MIRParametrisation& param, bool checkRotation) {
    const auto& user  = param.userParametrisation();
    const auto& field = param.fieldParametrisation();
    std::unique_ptr<const param::MIRParametrisation> same(new param::SameParametrisation(user, field, true));

    std::string interpolation;
    user.get("interpolation", interpolation);
    if (interpolation == "none") {
        return "";
    }

    const grid::Target target(param);

    bool forced = field.has("spectral") || option(user, "filter", false) || [&]() {
        std::vector<double> dummy;
        return checkRotation && target.rotated && !same->get("rotation", dummy);
    }();

    auto check_rotated_regular_ll = [&]() {
        if (target.rotated && user.has("area")) {
            std::string area_mode;
            param.get("area-mode", area_mode);

            if (area_mode == "mask") {
                throw exception::UserError("ECMWFStyle: option 'rotation' is incompatible with area mode 'mask'");
            }
        }
    };

    const std::string prefix(target.rotated ? "rotated-" : "");

    if (target.type == "regular-ll") {
        std::vector<double> grid_v;
        if (forced || !field.has("gridded_regular_ll") || !same->get("grid", grid_v) || !same_points(user, field)) {
            check_rotated_regular_ll();
            return prefix + target.type;
        }
        return "";
    }

    if (target.type == "namedgrid") {
        std::string field_grid;
        field.get("grid", field_grid);
        return forced || target.grid != field_grid ? prefix + target.type : "";
    }

    if (target.type == "reduced-gg") {
        long N = 0;
        return forced || !same->get("reduced", N) ? prefix + target.type : "";
    }

    if (target.type == "regular-gg") {
        long N = 0;
        return forced || !same->get("regular", N) ? prefix + target.type : "";
    }

    if (target.type == "octahedral-gg") {
        long N = 0;
        return forced || !same->get("octahedral", N) ? prefix + target.type : "";
    }

    if (target.type == "reduced-gg-pl-given") {
        std::vector<long> pl;
        return forced || !same->get("pl", pl) ? prefix + target.type : "";
    }

    if (target.type == "griddef") {
        if (target.rotated) {
            throw exception::UserError("ECMWFStyle: option 'rotation' is incompatible with 'griddef'");
        }
        return target.type;
    }

    if (target.type == "points") {
        if (user.has("latitudes") != user.has("longitudes")) {
            throw exception::UserError("ECMWFStyle: options 'latitudes' and 'longitudes' have to be provided together");
        }
        if (target.rotated) {
            throw exception::UserError(
                "ECMWFStyle: option 'rotation' is incompatible with 'latitudes' and 'longitudes'");
        }
        return target.type;
    }

    if (!target.type.empty()) {
        return prefix + target.type;
    }

    if (user.has("area")) {
        std::vector<double> grid_v;
        if (field.has("gridded_regular_ll") && same->get("grid", grid_v) && !same_points(user, field)) {
            check_rotated_regular_ll();
            return prefix + "regular-ll";
        }
    }

    if (target.rotated) {
        if (field.has("gridded_regular_ll") && !same_points(user, field)) {
            check_rotated_regular_ll();
            return prefix + "regular-ll";
        }
    }

    Log::debug() << "ECMWFStyle: did not determine target from parametrisation" << std::endl;
    return "";
}


static std::string intermediate_grid(const param::MIRParametrisation& param, const std::string& dfault) {
    std::string name;
    if (!param.get("intgrid", name) || name.empty()) {
        name = dfault;
    }

    std::unique_ptr<const intgrid::Intgrid> intermediate(intgrid::IntgridFactory::build(name, param));
    return intermediate->gridname();
}


// spectral truncation (from the Gaussian number N of the inverse transform grid) and filters
static void add_spectral_filters(action::ActionPlan& plan, const param::MIRParametrisation& param, long N) {
    const auto& user = param.userParametrisation();

    long inputTruncation = 0;
    ASSERT(param.fieldParametrisation().get("truncation", inputTruncation) && inputTruncation > 0);

    std::string name = "automatic";
    user.get("truncation", name);

    std::unique_ptr<const truncation::Truncation> rule(truncation::TruncationFactory::build(name, param, N));
    if (long T = 0; rule->truncation(T, inputTruncation)) {
        ASSERT(T > 0);
        plan.add("filter.sh-truncate", "truncation", T);
    }

    if (user.has("cesaro")) {
        plan.add("filter.sh-cesaro-summation-filter");
    }

    if (user.has("bandpass")) {
        plan.add("filter.sh-bandpass");
    }
}


static void add_formula(action::ActionPlan& plan, const param::MIRParametrisation& param,
                        const std::vector<std::string>& whens) {
    std::string formula;
    for (const auto& when : whens) {
        if (param.get("formula." + when, formula)) {
            std::string metadata;  // paramId for the results of formulas
            param.get("formula." + when + ".metadata", metadata);

            plan.add("calc.formula", "formula", formula, "formula.metadata", metadata);
            break;
        }
    }
}


ECMWFStyle::ECMWFStyle(const param::MIRParametrisation& parametrisation) : MIRStyle(parametrisation) {
    struct StyleParametrisation : public param::SimpleParametrisation {
        explicit StyleParametrisation(const eckit::PathName& path) {
            if (path.exists()) {
                if (auto value = eckit::YAMLParser::decodeFile(path); value.isMap()) {
                    // empty options are unset
                    util::ValueMap map(value);
                    for (auto it = map.begin(); it != map.end();) {
                        it = it->second.isNil() ? map.erase(it) : std::next(it);
                    }

                    map.set(*this);
                }
            }
        }
    } static const style(LibMir::configFile(LibMir::config_file::STYLE));

    // from the parametrisation if set, otherwise from the style configuration (unset or empty: first choice)
    auto style_option = [this](const std::string& key, const std::vector<std::string>& choices) {
        std::string value;
        if (!parametrisation_.get(key, value)) {
            style.get(key, value);
        }

        if (value.empty()) {
            return choices.front();
        }

        if (std::find(choices.begin(), choices.end(), value) == choices.end()) {
            throw exception::UserError("ECMWFStyle: option '" + key + "' invalid value '" + value +
                                       "', choices are: " + eckit::StringTools::join(", ", choices));
        }

        return value;
    };

    sh2gridWindCompatible_ = style_option("sh2grid-wind", {"default", "compatible"}) == "compatible";
    sh2gridIntgrid_        = style_option("sh2grid-intgrid", {"regular-gg-from-target", "compatible"}) == "compatible"
                                 ? "regular-gg-from-target-compatible"
                                 : "regular-gg-from-target";
}


void ECMWFStyle::prologue(action::ActionPlan& plan) const {
    const auto& user = parametrisation_.userParametrisation();

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

    bool resetMissingValues = false;
    parametrisation_.get("reset-missing-values", resetMissingValues);
    if (resetMissingValues) {
        plan.add("misc.reset-missing-values");
    }

    if (user.has("statistics") || user.has("input-statistics")) {
        plan.add("filter.statistics", "which-statistics", "input");
    }

    add_formula(plan, user, {"prologue"});
}


void ECMWFStyle::sh2grid(action::ActionPlan& plan) const {
    const auto& user = parametrisation_.userParametrisation();

    add_formula(plan, user, {"spectral", "raw"});

    long uv       = 0;
    bool uv_input = parametrisation_.fieldParametrisation().get("is_wind_component_uv", uv) && (uv != 0);

    bool rotation = user.has("rotation");
    bool vod2uv   = option(user, "vod2uv", false);
    bool uv2uv    = option(user, "uv2uv", false) || uv_input;  // where "MIR knowledge of winds" is hardcoded

    if (vod2uv && uv_input) {
        throw exception::UserError("ECMWFStyle: option 'vod2uv' is incompatible with input U/V");
    }

    if (vod2uv && uv2uv) {
        throw exception::UserError("ECMWFStyle: option 'vod2uv' is incompatible with option 'uv2uv'");
    }

    // inverse transform to the intermediate grid (if any), or the target grid
    const grid::Target target(parametrisation_);
    auto gridded = target_gridded_from_parametrisation(parametrisation_, false);
    auto intgrid = intermediate_grid(parametrisation_, sh2gridIntgrid_);

    add_spectral_filters(
        plan, parametrisation_,
        intgrid.empty() ? target.gaussianNumber : static_cast<long>(grid::Grid::lookup(intgrid).gaussianNumber()));

    const std::string transform = "transform." + std::string(vod2uv ? "sh-vod-to-uv-" : "sh-scalar-to-");

    if (!gridded.empty()) {
        if (intgrid.empty()) {
            plan.add(transform + gridded);

            if (uv2uv) {
                plan.add("filter.adjust-winds-scale-cos-latitude");
            }
        }
        else {
            plan.add(transform + "namedgrid", "grid", intgrid);

            // apply u = U / cos(theta) first, as the intermediate grid is typically Gaussian, hence avoiding bad
            // conditioning at the poles (compatible mode applies it after the interpolation)
            if (uv2uv && !sh2gridWindCompatible_) {
                plan.add("filter.adjust-winds-scale-cos-latitude");
            }

            if (target.rotated || target.grid != intgrid) {
                plan.add("interpolate.grid2" + gridded);
            }

            if (uv2uv && sh2gridWindCompatible_) {
                plan.add("filter.adjust-winds-scale-cos-latitude");
            }
        }

        if ((vod2uv || uv2uv) && rotation) {
            plan.add("filter.adjust-winds-directions");
        }
    }

    add_formula(plan, user, {"gridded"});
}


void ECMWFStyle::sh2sh(action::ActionPlan& plan) const {
    const auto& user = parametrisation_.userParametrisation();

    add_spectral_filters(plan, parametrisation_, grid::Target(parametrisation_).gaussianNumber);

    add_formula(plan, user, {"spectral", "raw"});

    bool vod2uv = option(user, "vod2uv", false);
    if (vod2uv) {
        plan.add("transform.sh-vod-to-UV");
    }
}


void ECMWFStyle::grid2grid(action::ActionPlan& plan) const {
    const auto& user  = parametrisation_.userParametrisation();
    const auto& field = parametrisation_.fieldParametrisation();

    bool rotation = user.has("rotation");
    bool vod2uv   = option(user, "vod2uv", false);
    bool uv2uv    = option(user, "uv2uv", false);

    if (vod2uv) {
        Log::error() << "ECMWFStyle: option 'vod2uv' does not support gridded input" << std::endl;
        ASSERT(!vod2uv);
    }

    add_formula(plan, user, {"gridded", "raw"});

    auto gridded = target_gridded_from_parametrisation(parametrisation_, rotation);
    if (!gridded.empty()) {
        std::string intgrid;
        if (std::string intint;
            parametrisation_.get("intermediate-interpolation", intint) && !intint.empty() && intint != "none") {
            if (intgrid = intermediate_grid(parametrisation_, "none"); !intgrid.empty()) {
                auto runtime = std::make_unique<param::RuntimeParametrisation>(parametrisation_);
                runtime->set("interpolation", intint);
                runtime->set("grid", intgrid);
                runtime->unset("rotation");

                param::CombinedParametrisation recombined(*runtime, field);
                if (auto inttarget = target_gridded_from_parametrisation(recombined, false); !inttarget.empty()) {
                    plan.add("interpolate.grid2" + inttarget, runtime.release());
                }
            }
        }

        // interpolate to the target grid, unless it is the intermediate grid
        if (const grid::Target target(parametrisation_); intgrid.empty() || target.rotated || target.grid != intgrid) {
            plan.add("interpolate.grid2" + gridded);
        }

        if (vod2uv || uv2uv) {
            ASSERT(vod2uv != uv2uv);

            if (rotation) {
                plan.add("filter.adjust-winds-directions");
            }
        }
    }
}


void ECMWFStyle::epilogue(action::ActionPlan& plan) const {
    const auto& user = parametrisation_.userParametrisation();

    bool vod2uv = option(user, "vod2uv", false);
    bool uv2uv  = option(user, "uv2uv", false);

    if (vod2uv || uv2uv) {
        ASSERT(vod2uv != uv2uv);

        bool u_only = option(user, "u-only", false);
        bool v_only = option(user, "v-only", false);

        if (u_only) {
            ASSERT(!v_only);
            plan.add("select.field", "which", 0L);
        }

        if (v_only) {
            ASSERT(!u_only);
            plan.add("select.field", "which", 1L);
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

    if (user.has("add-random")) {
        plan.add("filter.add-random");
    }

    if (user.has("limiter")) {
        plan.add("filter.limiter");
    }

    std::string epilogue;
    if (parametrisation_.get("epilogue", epilogue)) {
        plan.add(epilogue);
    }
}


void ECMWFStyle::print(std::ostream& out) const {
    out << "ECMWFStyle[]";
}


void ECMWFStyle::prepare(action::ActionPlan& plan, output::MIROutput& output) const {
    const auto& user = parametrisation_.userParametrisation();

    // All the nasty logic goes there
    prologue(plan);

    size_t user_wants_gridded = 0;

    if (user.has("grid")) {
        user_wants_gridded++;
    }

    if (user.has("gridname")) {
        static struct DeprecatedKeyword : util::DeprecatedFunctionality {
            DeprecatedKeyword() : util::DeprecatedFunctionality("keyword 'gridname' is now 'grid'") {}
        } __deprecated_gridname;
        user_wants_gridded++;
    }

    if (user.has("reduced")) {
        user_wants_gridded++;
    }

    if (user.has("regular")) {
        user_wants_gridded++;
    }

    if (user.has("octahedral")) {
        user_wants_gridded++;
    }

    if (user.has("pl")) {
        user_wants_gridded++;
    }

    if (user.has("griddef")) {
        user_wants_gridded++;
    }

    if (user.has("latitudes") || user.has("longitudes")) {
        user_wants_gridded++;
    }

    ASSERT(user_wants_gridded <= 1);

    if (option(user, "pre-globalise", false)) {
        plan.add("filter.globalise");
    }

    if (user.has("mask-input-lsm-value")) {
        plan.add("filter.mask-input-lsm");
    }

    bool field_gridded  = parametrisation_.fieldParametrisation().has("gridded");
    bool field_spectral = parametrisation_.fieldParametrisation().has("spectral");

    ASSERT(field_gridded != field_spectral);


    if (field_spectral) {
        if (user_wants_gridded > 0) {
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


    if (field_gridded || (user_wants_gridded > 0)) {
        if (std::string nabla; user.get("nabla", nabla)) {
            for (const auto& operation : eckit::StringTools::split("/", nabla)) {
                plan.add("filter." + operation);
            }
        }

        if (option(user, "globalise", false)) {
            plan.add("filter.globalise");
        }

        if (user.has("area")) {
            plan.add(key::Area::action(parametrisation_));
        }

        if (user.has("bitmap")) {
            plan.add("filter.bitmap");
        }

        if (user.has("mask-output-lsm-value")) {
            plan.add("filter.mask-output-lsm");
        }

        if (user.has("frame")) {
            plan.add("filter.frame");
        }

        if (user.has("unstructured")) {
            plan.add("filter.unstructured");
        }
    }


    epilogue(plan);


    output.prepare(parametrisation_, plan, output);

    ASSERT(plan.ended());
}


}  // namespace mir::key::style
