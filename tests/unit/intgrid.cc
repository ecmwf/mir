// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <string>
#include <vector>

#include "eckit/testing/Test.h"

#include "mir/key/grid/Grid.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/util/Exceptions.h"

#include "intgrid.h"


namespace mir::tests::unit {


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

        for (const std::string gridspec :
             {"{grid: 2/2}", "{grid: H8}", "{type: unstructured_ll, latitudes: [1, 2], longitudes: [3, 4]}"}) {
            auto t = make("grid", gridspec, none);
            EXPECT_EQUAL(t.type, "gridspec");
            EXPECT(!t.gaussian);
            EXPECT_EQUAL(t.gaussianNumber, N64);
        }
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


}  // namespace mir::tests::unit


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
