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
#include "mir/input/GribFileInput.h"
#include "mir/key/grid/Grid.h"
#include "mir/key/intgrid/Intgrid.h"
#include "mir/key/style/MIRStyle.h"
#include "mir/output/EmptyOutput.h"
#include "mir/param/CombinedParametrisation.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"


namespace mir::tests::unit {


using key::intgrid::Intgrid;
using key::intgrid::IntgridFactory;


// spectral input, at truncation T (Gaussian N = T + 1, cubic spectral order)
struct SpectralField : param::SimpleParametrisation {
    explicit SpectralField(long T, bool uv = false) {
        set("spectral", true).set("truncation", T);
        if (uv) {
            set("is_wind_component_uv", 1L);
        }
    }
};


struct GriddedField : param::SimpleParametrisation {
    GriddedField() { set("gridded", true).set("grid", std::vector<double>{2, 2}); }
};


key::grid::Target target(const param::MIRParametrisation& user, const param::MIRParametrisation& field) {
    const param::CombinedParametrisation param(user, field);
    return key::grid::Target(param);
}


// intermediate grid, as configured by 'intgrid'
std::string intgrid(const param::MIRParametrisation& user, const param::MIRParametrisation& field) {
    const param::CombinedParametrisation param(user, field);

    std::string name;
    ASSERT(user.get("intgrid", name));

    std::unique_ptr<const Intgrid> grid(IntgridFactory::build(name, param));
    return grid->gridname();
}


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


CASE("grid::Target") {
    auto make = [](const std::string& key, auto value, const param::MIRParametrisation& field) {
        param::SimpleParametrisation user;
        user.set(key, value);
        return target(user, field);
    };

    const param::SimpleParametrisation none;
    const SpectralField spectral(1279);
    const auto N64 = static_cast<long>(key::grid::Grid::default_gaussian_number());

    SECTION("not set") {
        auto t = target(none, none);
        EXPECT(t.type.empty() && t.grid.empty() && !t.gaussian && !t.rotated);
        EXPECT_EQUAL(t.gaussianNumber, 0);
    }

    SECTION("Gaussian grids") {
        struct test_t {
            std::string value;
            std::string expected;
            long N;
        };

        for (const auto& test : std::vector<test_t>{{"O320", "O320", 320}, {"n80", "N80", 80}, {"F16", "F16", 16}}) {
            auto t = make("grid", test.value, none);
            EXPECT_EQUAL(t.type, "namedgrid");
            EXPECT_EQUAL(t.grid, test.expected);
            EXPECT(t.gaussian && !t.rotated);
            EXPECT_EQUAL(t.gaussianNumber, test.N);
        }

        for (const auto& test : std::vector<test_t>{{"octahedral", "octahedral-gg", 200},
                                                    {"reduced", "reduced-gg", 200},
                                                    {"regular", "regular-gg", 200}}) {
            auto t = make(test.value, test.N, none);
            EXPECT_EQUAL(t.type, test.expected);
            EXPECT(t.gaussian && t.grid.empty());
            EXPECT_EQUAL(t.gaussianNumber, test.N);
        }

        auto pl = make("pl", std::vector<long>{20, 24, 24, 20}, none);
        EXPECT_EQUAL(pl.type, "reduced-gg-pl-given");
        EXPECT(pl.gaussian);
        EXPECT_EQUAL(pl.gaussianNumber, 0);
    }

    SECTION("Gaussian grids, by gridspec") {
        struct test_t {
            std::string gridspec;
            long N;
        };

        for (const auto& test : std::vector<test_t>{{"{grid: O32}", 32}, {"{grid: N48}", 48}, {"{grid: F16}", 16}}) {
            auto t = make("grid", test.gridspec, none);
            EXPECT_EQUAL(t.type, "gridspec");
            EXPECT(t.gaussian && !t.rotated);
            EXPECT_EQUAL(t.gaussianNumber, test.N);
        }

        auto regional = make("grid", "{grid: O32, area: [60, -10, 30, 40]}", none);
        EXPECT(regional.gaussian);
        EXPECT_EQUAL(regional.gaussianNumber, 32);

        // rotated: not suitable for a direct inverse spectral transform, Gaussian number still applies
        auto rotated = make("grid", "{grid: O32, projection: {type: rotation, south_pole: [-40, 22]}}", none);
        EXPECT(!rotated.gaussian);
        EXPECT_EQUAL(rotated.gaussianNumber, 32);
    }

    SECTION("non-Gaussian grids") {
        auto ll = make("grid", std::vector<double>{1, 1}, none);
        EXPECT_EQUAL(ll.type, "regular-ll");
        EXPECT(!ll.gaussian);
        EXPECT_EQUAL(ll.gaussianNumber, 90);

        auto healpix = make("grid", "H32", none);
        EXPECT_EQUAL(healpix.type, "namedgrid");
        EXPECT(!healpix.gaussian);
        EXPECT_EQUAL(healpix.gaussianNumber, 64);

        for (const std::string gridspec : {"{grid: 2/2}", "{grid: H8}"}) {
            auto t = make("grid", gridspec, none);
            EXPECT_EQUAL(t.type, "gridspec");
            EXPECT(!t.gaussian);
            EXPECT_EQUAL(t.gaussianNumber, N64);
        }

        auto griddef = make("griddef", "griddef", none);
        EXPECT_EQUAL(griddef.type, "griddef");
        EXPECT_EQUAL(griddef.gaussianNumber, N64);
    }

    SECTION("rotated") {
        param::SimpleParametrisation user;
        user.set("grid", "O320").set("rotation", std::vector<double>{-40, 22});

        auto t = target(user, none);
        EXPECT(t.gaussian && t.rotated);
    }

    SECTION("limited by spectral input truncation") {
        EXPECT_EQUAL(make("grid", "O320", spectral).gaussianNumber, 320);
        EXPECT_EQUAL(make("grid", "O2560", spectral).gaussianNumber, 1280);
        EXPECT_EQUAL(make("grid", "{grid: O2560}", spectral).gaussianNumber, 1280);
        EXPECT_EQUAL(make("pl", std::vector<long>{20, 24, 24, 20}, spectral).gaussianNumber, 1280);
        EXPECT_EQUAL(target(none, spectral).gaussianNumber, 1280);
    }
}


CASE("intgrid=regular-gg-from-target") {
    auto from_target = [](const param::MIRParametrisation& field, const std::string& key, auto value) {
        param::SimpleParametrisation user;
        user.set("intgrid", "regular-gg-from-target").set(key, value);
        return intgrid(user, field);
    };

    const SpectralField field(1279);

    SECTION("Gaussian target grids") {
        for (const std::string grid : {"O320", "N320", "F320", "source"}) {
            EXPECT_EQUAL(from_target(field, "grid", grid), "");
        }

        EXPECT_EQUAL(from_target(field, "octahedral", 320L), "");
        EXPECT_EQUAL(from_target(field, "reduced", 320L), "");
        EXPECT_EQUAL(from_target(field, "regular", 320L), "");
        EXPECT_EQUAL(from_target(field, "pl", std::vector<long>{20, 24, 24, 20}), "");
    }

    SECTION("rotated Gaussian target grid") {
        param::SimpleParametrisation user;
        user.set("intgrid", "regular-gg-from-target").set("grid", "O320").set("rotation", std::vector<double>{-40, 22});
        EXPECT_EQUAL(intgrid(user, field), "F320");
    }

    SECTION("Gaussian target grids, by gridspec") {
        EXPECT_EQUAL(from_target(field, "grid", "{grid: O320}"), "");
        EXPECT_EQUAL(from_target(field, "grid", "{grid: F320, area: [60, -10, 30, 40]}"), "");
        EXPECT_EQUAL(from_target(field, "grid", "{grid: O320, projection: {type: rotation, south_pole: [-40, 22]}}"),
                     "F320");
    }

    SECTION("non-Gaussian target grids") {
        EXPECT_EQUAL(from_target(field, "grid", std::vector<double>{1, 1}), "F90");
        EXPECT_EQUAL(from_target(field, "grid", "H32"), "F64");
        EXPECT_EQUAL(from_target(field, "grid", "{grid: 2/2}"), "F64");
    }

    SECTION("gridded input") {
        const GriddedField gridded;
        EXPECT_EQUAL(from_target(gridded, "grid", "O320"), "");
        EXPECT_EQUAL(from_target(gridded, "grid", std::vector<double>{1, 1}), "F90");
    }
}


CASE("intgrid=regular-gg-from-target-compatible (and aliases)") {
    const SpectralField field(1279);

    for (const std::string name : {"regular-gg-from-target-compatible", "automatic", "auto", "AUTO"}) {
        param::SimpleParametrisation user;
        user.set("intgrid", name);

        EXPECT_EQUAL(intgrid(user.set("grid", "O320"), field), "F320");
        EXPECT_EQUAL(intgrid(user.set("grid", std::vector<double>{1, 1}), field), "F90");
        EXPECT_EQUAL(intgrid(user.set("grid", "O320").set("rotation", std::vector<double>{-40, 22}), field), "F320");
    }
}


CASE("intgrid=none, named grids, source") {
    const SpectralField field(1279);

    param::SimpleParametrisation user;
    user.set("grid", "O320");

    for (const std::string name : {"none", "NONE"}) {
        EXPECT_EQUAL(intgrid(user.set("intgrid", name), field), "");
    }

    EXPECT_EQUAL(intgrid(user.set("intgrid", "O1280"), field), "O1280");
    EXPECT_EQUAL(intgrid(user.set("intgrid", "o1280"), field), "O1280");
    EXPECT_EQUAL(intgrid(user.set("intgrid", "F640"), field), "F640");
    EXPECT_EQUAL(intgrid(user.set("intgrid", "source"), field), "O1280");

    EXPECT_THROWS_AS(intgrid(user.set("intgrid", "source"), GriddedField()), exception::UserError);
    EXPECT_THROWS_AS(intgrid(user.set("intgrid", "?"), field), exception::SeriousBug);
}


CASE("grid2grid: intermediate grid") {
    const GriddedField field;

    param::SimpleParametrisation user;
    user.set("grid", std::vector<double>{1, 1});

    SECTION("intgrid unset") {
        Plan plan(user.set("intermediate-interpolation", "nn"), field);
        EXPECT(plan.find("Gridded2RegularLL") == 0);
        EXPECT(plan.find("Save") == 1);
    }

    SECTION("intgrid=none") {
        Plan plan(user.set("intgrid", "none").set("intermediate-interpolation", "nn"), field);
        EXPECT(plan.find("Gridded2RegularLL") == 0);
        EXPECT(plan.find("Save") == 1);
    }

    SECTION("intgrid=O32, intermediate-interpolation unset, empty or none") {
        user.set("intgrid", "O32");

        Plan unset(user, field);
        EXPECT(unset.find("Gridded2RegularLL") == 0);
        EXPECT(unset.find("Save") == 1);

        for (const std::string intint : {"", "none"}) {
            Plan plan(user.set("intermediate-interpolation", intint), field);
            EXPECT(plan.find("Gridded2RegularLL") == 0);
            EXPECT(plan.find("Save") == 1);
        }
    }

    SECTION("intgrid=O32, intermediate-interpolation=nn") {
        Plan plan(user.set("intgrid", "O32").set("intermediate-interpolation", "nn"), field);
        EXPECT(plan.find("Gridded2NamedGrid", {"grid=O32,interpolation=nn,"}) == 0);
        EXPECT(plan.find("Gridded2RegularLL", {"interpolation=linear,"}) == 1);
        EXPECT(plan.find("Save") == 2);
    }

    SECTION("intgrid=regular-gg-from-target, intermediate-interpolation=nn") {
        user.set("intgrid", "regular-gg-from-target").set("intermediate-interpolation", "nn");

        Plan plan(user, field);
        EXPECT(plan.find("Gridded2NamedGrid", {"grid=F90,interpolation=nn,"}) == 0);
        EXPECT(plan.find("Gridded2RegularLL") == 1);
        EXPECT(plan.find("Save") == 2);

        Plan direct(user.set("grid", "O320"), field);
        EXPECT(direct.find("Gridded2NamedGrid", {"grid=O320,interpolation=linear,"}) == 0);
        EXPECT(direct.find("Save") == 1);
    }

    SECTION("intgrid=O32, intermediate-interpolation=nn, target grid O32") {
        user.set("grid", "O32").set("intgrid", "O32").set("intermediate-interpolation", "nn");

        Plan plan(user, field);
        EXPECT(plan.find("Gridded2NamedGrid", {"grid=O32,interpolation=nn,"}) == 0);
        EXPECT(plan.find("Save") == 1);
    }

    SECTION("intgrid=O32, intermediate-interpolation=nn, target grid O32 rotated") {
        user.set("grid", "O32").set("rotation", std::vector<double>{-40, 22});

        Plan plan(user.set("intgrid", "O32").set("intermediate-interpolation", "nn"), field);
        EXPECT(plan.find("Gridded2NamedGrid", {"grid=O32,interpolation=nn,"}) == 0);
        EXPECT(plan.find("Gridded2RotatedNamedGrid", {"grid=O32,"}) == 1);
    }

    SECTION("intgrid=source, intermediate-interpolation=nn") {
        user.set("intgrid", "source").set("intermediate-interpolation", "nn");
        EXPECT_THROWS_AS(Plan plan(user, field), exception::UserError);
    }
}


CASE("sh2grid: style option sh2grid-intgrid") {
    const SpectralField field(1279);

    param::SimpleParametrisation user;
    user.set("grid", "O320");

    SECTION("regular-gg-from-target (default, unset or empty)") {
        Plan plan(user, field);
        EXPECT(plan.has("ShToNamedGrid", {"grid=O320"}));
        EXPECT(!plan.interpolates());

        for (const std::string value : {"regular-gg-from-target", ""}) {
            Plan direct(user.set("sh2grid-intgrid", value), field);
            EXPECT(direct.has("ShToNamedGrid", {"grid=O320"}));
            EXPECT(!direct.interpolates());
        }
    }

    SECTION("compatible") {
        Plan plan(user.set("sh2grid-intgrid", "compatible"), field);
        EXPECT(plan.has("ShTruncate", {"truncation=639"}));
        EXPECT(plan.has("ShToNamedGrid", {"grid=F320"}));
        EXPECT(plan.has("Gridded2NamedGrid", {"grid=O320"}));
    }

    SECTION("compatible, with intgrid set") {
        Plan plan(user.set("sh2grid-intgrid", "compatible").set("intgrid", "regular-gg-from-target"), field);
        EXPECT(plan.has("ShToNamedGrid", {"grid=O320"}));
        EXPECT(!plan.interpolates());
    }

    SECTION("invalid") {
        user.set("sh2grid-intgrid", "?");
        EXPECT_THROWS_AS(Plan plan(user, field), exception::UserError);
    }

    SECTION("GRIB input") {
        std::unique_ptr<input::MIRInput> input(new input::GribFileInput("mtg2-t.grib2"));
        ASSERT(input->next());

        const auto& grib = input->parametrisation();

        Plan plan(user, grib);
        EXPECT(plan.has("ShToNamedGrid", {"grid=O320"}));
        EXPECT(!plan.interpolates());

        Plan compatible(user.set("sh2grid-intgrid", "compatible"), grib);
        EXPECT(compatible.has("ShToNamedGrid", {"grid=F"}));
        EXPECT(compatible.has("Gridded2NamedGrid", {"grid=O320"}));
    }
}


CASE("sh2grid: spectral to Gaussian grids, directly (default)") {
    const SpectralField field(1279);

    SECTION("octahedral, named") {
        param::SimpleParametrisation user;
        user.set("grid", "O320");

        Plan plan(user, field);
        EXPECT(plan.has("ShTruncate", {"truncation=639"}));
        EXPECT(plan.has("ShToNamedGrid", {"invtrans=<scalar>", "grid=O320"}));
        EXPECT(!plan.interpolates());
    }

    SECTION("octahedral, named (lowercase)") {
        param::SimpleParametrisation user;
        user.set("grid", "o320");

        Plan plan(user, field);
        EXPECT(plan.has("ShToNamedGrid", {"grid=O320"}));
        EXPECT(!plan.interpolates());
    }

    SECTION("reduced classic, named") {
        param::SimpleParametrisation user;
        user.set("grid", "N320");

        Plan plan(user, field);
        EXPECT(plan.has("ShTruncate", {"truncation=639"}));
        EXPECT(plan.has("ShToNamedGrid", {"grid=N320"}));
        EXPECT(!plan.interpolates());
    }

    SECTION("regular, named") {
        param::SimpleParametrisation user;
        user.set("grid", "F320");

        Plan plan(user, field);
        EXPECT(plan.has("ShTruncate", {"truncation=639"}));
        EXPECT(plan.has("ShToNamedGrid", {"grid=F320"}));
        EXPECT(!plan.interpolates());
    }

    SECTION("octahedral, by Gaussian number") {
        param::SimpleParametrisation user;
        user.set("octahedral", 320L);

        Plan plan(user, field);
        EXPECT(plan.has("ShTruncate", {"truncation=639"}));
        EXPECT(plan.has("ShToOctahedralGG"));
        EXPECT(!plan.has("ShToNamedGrid"));
        EXPECT(!plan.interpolates());
    }

    SECTION("reduced classic, by Gaussian number") {
        param::SimpleParametrisation user;
        user.set("reduced", 320L);

        Plan plan(user, field);
        EXPECT(plan.has("ShToReducedGG"));
        EXPECT(!plan.has("ShToNamedGrid"));
        EXPECT(!plan.interpolates());
    }

    SECTION("regular, by Gaussian number") {
        param::SimpleParametrisation user;
        user.set("regular", 320L);

        Plan plan(user, field);
        EXPECT(plan.has("ShToRegularGG"));
        EXPECT(!plan.has("ShToNamedGrid"));
        EXPECT(!plan.interpolates());
    }

    SECTION("reduced, by pl") {
        param::SimpleParametrisation user;
        user.set("pl", std::vector<long>{20, 24, 24, 20});

        Plan plan(user, field);
        EXPECT(plan.has("ShToReducedGGPLGiven"));
        EXPECT(!plan.has("ShToNamedGrid"));
        EXPECT(!plan.interpolates());
    }

    SECTION("gridspec") {
        param::SimpleParametrisation user;
        user.set("grid", "{grid: O320}");

        Plan plan(user, field);
        EXPECT(plan.has("ShTruncate", {"truncation=639"}));
        EXPECT(plan.has("ShToGridSpec", {"cropping=none", R"(gridspec={"grid":"O320"})"}));
        EXPECT(!plan.interpolates());
    }

    SECTION("gridspec, regional (global grid, cropped)") {
        param::SimpleParametrisation user;
        user.set("grid", "{grid: O320, area: [60, -10, 30, 40]}");

        Plan plan(user, field);
        EXPECT(plan.has("ShTruncate", {"truncation=639"}));
        EXPECT(plan.has("ShToGridSpec", {"cropping=BoundingBox[", R"(gridspec={"grid":"O320"})"}));
        EXPECT(!plan.interpolates());
    }

    SECTION("source") {
        param::SimpleParametrisation user;
        user.set("grid", "source");

        Plan plan(user, field);
        EXPECT(plan.has("ShToNamedGrid", {"grid=O1280"}));
        EXPECT(!plan.interpolates());
    }

    SECTION("target grid finer than input truncation") {
        param::SimpleParametrisation user;
        user.set("grid", "O1280");

        Plan plan(user, SpectralField(639));
        EXPECT(!plan.has("ShTruncate"));
        EXPECT(plan.has("ShToNamedGrid", {"grid=O1280"}));
        EXPECT(!plan.interpolates());
    }

    SECTION("area") {
        param::SimpleParametrisation user;
        user.set("grid", "O320").set("area", std::vector<double>{60, -10, 30, 40});

        Plan plan(user, field);
        EXPECT(plan.has("ShToNamedGrid", {"grid=O320"}));
        EXPECT(plan.has("AreaCropper"));
        EXPECT(!plan.interpolates());
    }

    SECTION("vod2uv") {
        param::SimpleParametrisation user;
        user.set("grid", "O320").set("vod2uv", true);

        Plan plan(user, field);
        EXPECT(plan.has("ShToNamedGrid", {"invtrans=<vod2uv>", "grid=O320"}));
        EXPECT(!plan.has("AdjustWindsScaleCosLatitude"));
        EXPECT(!plan.interpolates());
    }

    SECTION("U/V input") {
        param::SimpleParametrisation user;
        user.set("grid", "O320");

        Plan plan(user, SpectralField(1279, true));
        auto transform = plan.find("ShToNamedGrid", {"invtrans=<scalar>", "grid=O320"});
        auto scale     = plan.find("AdjustWindsScaleCosLatitude");

        EXPECT(transform != Plan::npos);
        EXPECT(scale != Plan::npos);
        EXPECT(transform < scale);
        EXPECT(!plan.interpolates());
    }

    SECTION("intgrid=none") {
        param::SimpleParametrisation user;
        user.set("grid", "O320").set("intgrid", "none");

        Plan plan(user, field);
        EXPECT(plan.has("ShToNamedGrid", {"grid=O320"}));
        EXPECT(!plan.interpolates());
    }
}


CASE("sh2grid: spectral to Gaussian grids, with intermediate grid") {
    const SpectralField field(1279);

    SECTION("intgrid=O640") {
        param::SimpleParametrisation user;
        user.set("grid", "O320").set("intgrid", "O640");

        Plan plan(user, field);
        auto transform   = plan.find("ShToNamedGrid", {"grid=O640"});
        auto interpolate = plan.find("Gridded2NamedGrid", {"grid=O320"});

        EXPECT(transform != Plan::npos);
        EXPECT(interpolate != Plan::npos);
        EXPECT(transform < interpolate);
    }

    SECTION("intgrid=O320 (the target grid)") {
        param::SimpleParametrisation user;
        user.set("grid", "o320").set("intgrid", "O320");

        Plan plan(user, field);
        EXPECT(plan.has("ShToNamedGrid", {"grid=O320"}));
        EXPECT(!plan.interpolates());
    }

    SECTION("intgrid=source") {
        param::SimpleParametrisation user;
        user.set("grid", "O320").set("intgrid", "source");

        Plan plan(user, field);
        EXPECT(plan.has("ShToNamedGrid", {"grid=O1280"}));
        EXPECT(plan.has("Gridded2NamedGrid", {"grid=O320"}));
    }

    SECTION("rotated") {
        param::SimpleParametrisation user;
        user.set("grid", "O320").set("rotation", std::vector<double>{-40, 22});

        Plan plan(user, field);
        EXPECT(plan.has("ShToNamedGrid", {"grid=F320"}));
        EXPECT(plan.has("Gridded2RotatedNamedGrid", {"grid=O320"}));
    }

    SECTION("gridspec, rotated") {
        param::SimpleParametrisation user;
        user.set("grid", "{grid: O320, projection: {type: rotation, south_pole: [-40, 22]}}");

        Plan plan(user, field);
        EXPECT(plan.has("ShTruncate", {"truncation=639"}));
        EXPECT(plan.has("ShToNamedGrid", {"grid=F320"}));
        EXPECT(plan.has("Gridded2GridSpec"));
    }

    SECTION("intgrid=regular-gg-from-target-compatible (and aliases)") {
        struct test_t {
            std::string key;
            std::string value;
            std::string interpolate;
        };

        for (const std::string intgrid : {"regular-gg-from-target-compatible", "automatic", "auto", "AUTO"}) {
            for (const auto& test : std::vector<test_t>{
                     {"grid", "O320", "Gridded2NamedGrid"},
                     {"grid", "N320", "Gridded2NamedGrid"},
                     {"octahedral", "320", "Gridded2OctahedralGG"},
                     {"reduced", "320", "Gridded2ReducedGG"},
                     {"regular", "320", "Gridded2RegularGG"},
                 }) {
                param::SimpleParametrisation user;
                user.set("intgrid", intgrid);
                if (test.key == "grid") {
                    user.set(test.key, test.value);
                }
                else {
                    user.set(test.key, std::stol(test.value));
                }

                Plan plan(user, field);
                auto transform   = plan.find("ShToNamedGrid", {"grid=F320"});
                auto interpolate = plan.find(test.interpolate);

                EXPECT(plan.has("ShTruncate", {"truncation=639"}));
                EXPECT(transform != Plan::npos);
                EXPECT(interpolate != Plan::npos);
                EXPECT(transform < interpolate);
            }
        }
    }

    SECTION("intgrid=regular-gg-from-target-compatible, gridspec") {
        param::SimpleParametrisation user;
        user.set("grid", "{grid: O320}").set("intgrid", "regular-gg-from-target-compatible");

        Plan plan(user, field);
        EXPECT(plan.has("ShTruncate", {"truncation=639"}));
        EXPECT(plan.has("ShToNamedGrid", {"grid=F320"}));
        EXPECT(plan.has("Gridded2GridSpec"));
    }

    SECTION("intgrid=regular-gg-from-target-compatible, F320 (the target grid)") {
        param::SimpleParametrisation user;
        user.set("grid", "F320").set("intgrid", "regular-gg-from-target-compatible");

        Plan plan(user, field);
        EXPECT(plan.has("ShToNamedGrid", {"grid=F320"}));
        EXPECT(!plan.interpolates());
    }

    SECTION("intgrid=regular-gg-from-target-compatible, target grid finer than input truncation") {
        param::SimpleParametrisation user;
        user.set("grid", "O1280").set("intgrid", "regular-gg-from-target-compatible");

        Plan plan(user, SpectralField(639));
        EXPECT(plan.has("ShToNamedGrid", {"grid=F640"}));
        EXPECT(plan.has("Gridded2NamedGrid", {"grid=O1280"}));
    }
}


CASE("sh2grid: spectral to non-Gaussian grids, with intermediate grid") {
    const SpectralField field(1279);

    SECTION("regular lat/lon") {
        param::SimpleParametrisation user;
        user.set("grid", std::vector<double>{1, 1});

        Plan plan(user, field);
        EXPECT(plan.has("ShToNamedGrid", {"grid=F90"}));
        EXPECT(plan.has("Gridded2RegularLL"));
    }

    SECTION("HEALPix") {
        param::SimpleParametrisation user;
        user.set("grid", "H32");

        Plan plan(user, field);
        EXPECT(plan.has("ShToNamedGrid", {"grid=F64"}));
        EXPECT(plan.has("Gridded2NamedGrid", {"grid=H32"}));
    }
}


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
        EXPECT(plan.has("ShToNamedGrid", {"grid=O320"}));
        EXPECT(!plan.interpolates());
    }
}


}  // namespace mir::tests::unit


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
