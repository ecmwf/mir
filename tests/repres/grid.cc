// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <functional>
#include <sstream>
#include <string>
#include <vector>

#include "eckit/testing/Test.h"

#include "mir/key/grid/Grid.h"
#include "mir/key/grid/GridPattern.h"
#include "mir/param/CombinedParametrisation.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/repres/latlon/RegularLL.h"
#include "mir/util/Rotation.h"

#include "repres.h"


namespace mir::repres::tests {


using key::grid::Grid;


CASE("Grid: by name, pattern or specification") {
    struct test_t {
        std::string key;
        std::string type;
        bool gaussian;
        size_t gaussianNumber;
    };

    const std::vector<test_t> tests{
        {"O32", "namedgrid", true, 32},
        {"o32", "namedgrid", true, 32},
        {"N32", "namedgrid", true, 32},
        {"F32", "namedgrid", true, 32},
        {"1/1", "regular-ll", false, 90},
        {"0.5/2", "regular-ll", false, 45},
        {"{grid: O32}", "gridspec", true, 32},
        {"{grid: F32}", "gridspec", true, 32},
        {"{grid: O32, rotation: [-40, 22]}", "gridspec", false, 32},
        {"{grid: [1, 1]}", "gridspec", false, Grid::default_gaussian_number()},
        {"{projection: {type: lcc, lat_0: 50, lat_1: 50, lon_0: 10, reverse: true}, x: [-1707818.21687531, "
         "192181.783124688, 100000], y: [-887705.800790069, -2287705.80079007, -100000]}",
         "gridspec", false, Grid::default_gaussian_number()},
    };

    for (const auto& test : tests) {
        SECTION(test.key) {
            const auto& grid = Grid::lookup(test.key);

            EXPECT_EQUAL(grid.type(), test.type);
            EXPECT_EQUAL(grid.isGaussian(), test.gaussian);
            EXPECT_EQUAL(grid.gaussianNumber(), test.gaussianNumber);

            // registered by key (a canonical key for patterns)
            EXPECT(&Grid::lookup(grid.key()) == &grid);

            std::ostringstream str;
            str << grid;
            EXPECT(!str.str().empty());
        }
    }
}


CASE("Grid: representations") {
    const util::Rotation rotation(-40., 22.);


    SECTION("named Gaussian grids") {
        const auto& grid = Grid::lookup("O32");
        EXPECT_EQUAL(grid.gridname(), "O32");

        RepresentationHandle repres(grid.representation());
        EXPECT(repres->uniqueName().find("O32") == 0);

        RepresentationHandle rotated(grid.representation(rotation));
        EXPECT_EQUAL(rotated->numberOfPoints(), repres->numberOfPoints());
    }


    SECTION("regular lat/lon grids") {
        const auto& grid = Grid::lookup("1/1");

        RepresentationHandle repres(grid.representation());
        const auto& ll = dynamic_cast<const latlon::RegularLL&>(*repres);
        EXPECT_EQUAL(ll.Ni(), 360);
        EXPECT_EQUAL(ll.Nj(), 181);

        // not named, and not rotated
        EXPECT_THROWS_AS(grid.gridname(), exception::SeriousBug);
        EXPECT_THROWS_AS(grid.representation(rotation), exception::SeriousBug);
    }


    SECTION("grids by specification (as gridSpec, not as representation)") {
        const auto& grid = Grid::lookup("{grid: O32}");
        EXPECT_THROWS_AS(grid.representation(), exception::SeriousBug);
    }
}


CASE("Grid: canonical names, lookup and list") {
    param::SimpleParametrisation user;


    SECTION("canonical") {
        struct test_t {
            std::string name;
            std::string canonical;
        };

        const std::vector<test_t> tests{
            {"o32", "O32"},
            {"n32", "N32"},
            {"f32", "F32"},
            {"2/2", "2/2"},
            {"0.50/1.0", "0.5/1"},
            {"unknown", ""},
            {"{grid: [1, 1]}", R"({"grid":[1,1]})"},
        };

        for (const auto& test : tests) {
            EXPECT_EQUAL(Grid::canonical(test.name, user), test.canonical);
        }

        // typed grids, replaced by gridspecs (an unknown grid otherwise)
        EXPECT_THROWS_AS(Grid::canonical("gridType=lambert;Ni=20;Nj=15", user), exception::UserError);
        EXPECT_EQUAL(Grid::canonical("gridType=polar_stereographic", user), "");
    }


    SECTION("source (the intermediate grid of a spectral field)") {
        param::SimpleParametrisation field;
        param::CombinedParametrisation without(user, field, field);
        EXPECT_EQUAL(Grid::canonical("source", without), "");

        field.set("truncation", 63L);
        param::CombinedParametrisation with(user, field, field);
        EXPECT_EQUAL(Grid::canonical("source", with), "O64");
        EXPECT_EQUAL(Grid::canonical("SOURCE", with), "O64");
    }


    SECTION("get (canonical, if set and known)") {
        param::SimpleParametrisation param;

        std::string value;
        EXPECT_NOT(Grid::get("grid", value, param));

        param.set("grid", "o16");
        EXPECT(Grid::get("grid", value, param));
        EXPECT_EQUAL(value, "O16");

        param.set("grid", "unknown");
        EXPECT_NOT(Grid::get("grid", value, param));
    }


    SECTION("lookup (unknown) and list") {
        EXPECT_THROWS_AS(Grid::lookup("unknown"), exception::SeriousBug);

        (void)Grid::lookup("O48");

        std::ostringstream str;
        Grid::list(str);
        EXPECT(str.str().find("O48") != std::string::npos);

        std::ostringstream patterns;
        key::grid::GridPattern::list(patterns);
        EXPECT(patterns.str().find("^[oO][1-9][0-9]*$") != std::string::npos);

        EXPECT_THROWS_AS(key::grid::GridPattern::lookup("unknown"), exception::SeriousBug);
        EXPECT_EQUAL(key::grid::GridPattern::match("unknown"), "");
    }
}


CASE("Grid: target (by user and field parametrisations)") {
    using set_t = std::function<void(param::SimpleParametrisation&)>;

    struct test_t {
        std::string name;
        set_t user;
        long truncation;  // of a spectral field, if > 0
        std::string type;
        std::string grid;
        bool gaussian;
        bool rotated;
        long gaussianNumber;
    };

    auto grid = [](const std::string& name) {
        return set_t([name](param::SimpleParametrisation& user) { user.set("grid", name); });
    };

    auto N = [](const std::string& key) {
        return set_t([key](param::SimpleParametrisation& user) { user.set(key, 32L); });
    };

    const std::vector<test_t> tests{
        {"grid (named)", grid("o32"), 0, "namedgrid", "O32", true, false, 32},
        {"grid (named), rotated",
         [](param::SimpleParametrisation& user) {
             user.set("grid", "O32");
             user.set("rotation", std::vector<double>{-40., 22.});
         },
         0, "namedgrid", "O32", true, true, 32},
        {"grid (regular lat/lon)", grid("1/1"), 0, "regular-ll", "1/1", false, false, 90},
        {"grid (unknown)", grid("unknown"), 0, "", "", false, false, 0},
        {"reduced", N("reduced"), 0, "reduced-gg", "", true, false, 32},
        {"regular", N("regular"), 0, "regular-gg", "", true, false, 32},
        {"octahedral", N("octahedral"), 0, "octahedral-gg", "", true, false, 32},
        {"pl", [](param::SimpleParametrisation& user) { user.set("pl", std::vector<long>{20, 24, 24, 20}); }, 0,
         "reduced-gg-pl-given", "", true, false, 0},
        {"latitudes", [](param::SimpleParametrisation& user) { user.set("latitudes", std::vector<double>{1., 2.}); }, 0,
         "points", "", false, false, 64},
        {"grid, from spectral (limited by the truncation)", grid("O32"), 21, "namedgrid", "O32", true, false, 22},
        {"grid, from spectral", grid("O32"), 63, "namedgrid", "O32", true, false, 32},
        {"spectral only", [](param::SimpleParametrisation&) {}, 63, "", "", false, false, 64},
    };

    for (const auto& test : tests) {
        SECTION(test.name) {
            param::SimpleParametrisation user;
            test.user(user);

            param::SimpleParametrisation field;
            if (test.truncation > 0) {
                field.set("spectral", true);
                field.set("truncation", test.truncation);
            }

            param::CombinedParametrisation param(user, field, field);
            const key::grid::Target target(param);

            EXPECT_EQUAL(target.type, test.type);
            EXPECT_EQUAL(target.grid, test.grid);
            EXPECT_EQUAL(target.gaussian, test.gaussian);
            EXPECT_EQUAL(target.rotated, test.rotated);
            EXPECT_EQUAL(target.gaussianNumber, test.gaussianNumber);
        }
    }
}


}  // namespace mir::repres::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
