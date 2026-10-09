// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <algorithm>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "eckit/testing/Test.h"
#include "eckit/utils/StringTools.h"

#include "mir/action/plan/Action.h"
#include "mir/action/plan/ActionPlan.h"
#include "mir/key/style/MIRStyle.h"
#include "mir/output/EmptyOutput.h"
#include "mir/param/CombinedParametrisation.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"


namespace mir::tests::unit {


// spectral input, at truncation T (Gaussian N = T + 1, cubic spectral order)
struct SpectralField : param::SimpleParametrisation {
    explicit SpectralField(long T, bool uv = false) {
        set("spectral", true).set("truncation", T);
        if (uv) {
            set("is_wind_component_uv", 1L);
        }
    }
};


// plan (printed actions) from post-processing a field, as prepared by the default style
struct Plan {
    Plan(const param::MIRParametrisation& user, const param::MIRParametrisation& field) {
        const param::CombinedParametrisation param(user, field);
        std::unique_ptr<key::style::MIRStyle> style(key::style::MIRStyleFactory::build(param));

        output::EmptyOutput out;
        action::ActionPlan plan(param);
        style->prepare(plan, out);

        for (const auto* action : plan) {
            ASSERT(action != nullptr);
            std::ostringstream str;
            str << *action;
            actions_.emplace_back(str.str());
        }

        Log::info() << "user: " << user << "\nplan: " << plan << std::endl;
    }

    // position of first action starting with prefix, and containing all of the (optional) substrings
    size_t find(const std::string& prefix, const std::vector<std::string>& contains = {}) const {
        auto it = std::find_if(actions_.begin(), actions_.end(), [&](const std::string& action) {
            return eckit::StringTools::startsWith(action, prefix + "[") &&
                   std::all_of(contains.begin(), contains.end(), [&](const std::string& substring) {
                       return action.find(substring) != std::string::npos;
                   });
        });
        return it == actions_.end() ? npos : static_cast<size_t>(it - actions_.begin());
    }

    bool has(const std::string& prefix, const std::vector<std::string>& contains = {}) const {
        return find(prefix, contains) != npos;
    }

    // interpolation, from gridded to gridded
    bool interpolates() const {
        return std::any_of(actions_.begin(), actions_.end(), [](const std::string& action) {
            return eckit::StringTools::startsWith(action, "Gridded2");
        });
    }

    static constexpr size_t npos = std::string::npos;

private:
    std::vector<std::string> actions_;
};


CASE("sh2grid: spectral truncation, from the inverse transform grid") {
    const SpectralField field(1279);

    param::SimpleParametrisation user;
    user.set("grid", "O320");

    SECTION("target grid") {
        Plan plan(user, field);
        EXPECT(plan.has("ShTruncate", {"truncation=639"}));
    }

    SECTION("intermediate grid") {
        Plan coarser(user.set("intgrid", "F160"), field);
        EXPECT(coarser.has("ShTruncate", {"truncation=319"}));
        EXPECT(coarser.has("ShToNamedGrid", {"grid=F160"}));
        EXPECT(coarser.has("Gridded2NamedGrid", {"grid=O320"}));

        Plan finer(user.set("intgrid", "O640"), field);
        EXPECT(!finer.has("ShTruncate"));
        EXPECT(finer.has("ShToNamedGrid", {"grid=O640"}));
    }

    SECTION("spectral-order=cubic") {
        Plan plan(user.set("spectral-order", "cubic"), field);
        EXPECT(plan.has("ShTruncate", {"truncation=319"}));
    }

    SECTION("truncation=213") {
        Plan plan(user.set("truncation", "213"), field);
        EXPECT(plan.has("ShTruncate", {"truncation=213"}));
    }

    SECTION("truncation=none") {
        Plan plan(user.set("truncation", "none"), field);
        EXPECT(!plan.has("ShTruncate"));
    }

    SECTION("spectral output") {
        param::SimpleParametrisation spectral;

        Plan plan(spectral, field);
        EXPECT(!plan.has("ShTruncate"));

        Plan truncated(spectral.set("truncation", "639"), field);
        EXPECT(truncated.has("ShTruncate", {"truncation=639"}));
        EXPECT(!truncated.has("ShToNamedGrid"));
    }
}


CASE("sh2grid: U/V input, wind scaling by cos(latitude) and intermediate grid") {
    const SpectralField field(1279, true);

    // empty option is the default
    for (const std::string wind : {"default", "compatible", ""}) {
        param::SimpleParametrisation user;
        user.set("grid", std::vector<double>{1, 1}).set("sh2grid-wind", wind);

        Plan plan(user, field);
        auto transform   = plan.find("ShToNamedGrid");
        auto scale       = plan.find("AdjustWindsScaleCosLatitude");
        auto interpolate = plan.find("Gridded2RegularLL");

        EXPECT(transform != Plan::npos);
        EXPECT(scale != Plan::npos);
        EXPECT(interpolate != Plan::npos);
        EXPECT(transform < scale && transform < interpolate);

        // wind scaling on the (Gaussian) intermediate grid, compatible mode on the target grid
        EXPECT(wind == "compatible" ? interpolate < scale : scale < interpolate);
        EXPECT(!plan.has("AdjustWindsDirections"));
    }
}


CASE("sh2grid: U/V input, rotated") {
    const SpectralField field(1279, true);

    for (const std::string wind : {"default", "compatible", ""}) {
        param::SimpleParametrisation user;
        user.set("grid", "O320").set("rotation", std::vector<double>{-40, 22}).set("sh2grid-wind", wind);

        Plan plan(user, field);
        auto scale      = plan.find("AdjustWindsScaleCosLatitude");
        auto directions = plan.find("AdjustWindsDirections");

        EXPECT(scale != Plan::npos);
        EXPECT(directions != Plan::npos);
        EXPECT(scale < directions);
    }
}


CASE("sh2grid: incompatible options") {
    param::SimpleParametrisation user;
    user.set("grid", "O320").set("vod2uv", true);

    SECTION("vod2uv and U/V input") {
        EXPECT_THROWS_AS(Plan plan(user, SpectralField(1279, true)), exception::UserError);
    }

    SECTION("vod2uv and uv2uv") {
        user.set("uv2uv", true);
        EXPECT_THROWS_AS(Plan plan(user, SpectralField(1279)), exception::UserError);
    }
}


CASE("sh2grid: style option sh2grid-wind") {
    const SpectralField field(1279);

    param::SimpleParametrisation user;
    user.set("grid", "O320");

    SECTION("invalid") {
        user.set("sh2grid-wind", "?");
        EXPECT_THROWS_AS(Plan plan(user, field), exception::UserError);
    }

    SECTION("empty is the default") {
        user.set("sh2grid-wind", "");

        Plan plan(user, field);
        EXPECT(plan.has("ShToNamedGrid"));
    }
}


}  // namespace mir::tests::unit


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
