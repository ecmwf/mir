// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "eckit/geo/eckit_geo_config.h"

#include "regression.h"


namespace mir::tests::regressions {


CASE("to healpix: grid-box-average") {
    // MIR-632
    // input gridType=healpix,Nside=32,orderingConvention=ring.grib2, as its grid
    auto h = run(
        "{artificialInput:gridspec,gridSpec:{grid:H32}}", {{"interpolation", "grid-box-average"}, {"grid", "H36"}},
        R"([{"type":"interpolate.grid2namedgrid","interpolation":"grid-box-average","lsm-weight-adjustment":0.2,"non-linear":["missing-if-heaviest-missing"],"pole-displacement-in-degree":0,"prune-epsilon":1e-10,"solver":{"type":"multiply"},"grid":"H36"},{"type":"save","packing":"ccsds","edition":2}])",
        R"([{"grid":"H32"},{"grid":"H36"},{"grid":"H36"}])");

    EXPECT_EQUAL(get(h, "gridType"), "healpix");
    EXPECT_EQUAL(get(h, "gridName"), "H36");
    EXPECT_EQUAL(get(h, "Nside"), "36");
    EXPECT_EQUAL(get(h, "numberOfDataPoints"), "15552");
    EXPECT_EQUAL(get(h, "longitudeOfFirstGridPointInDegrees"), "45");
}


CASE("to healpix: healpix-resample") {
    SECTION("001") {
        // MIR-708
        // input gridType=healpix,Nside=2,orderingConvention=ring.grib2, as its grid
        auto h = run(
            "{artificialInput:gridspec,gridSpec:{grid:H2}}", {{"grid", "hn2"}, {"interpolation", "healpix-resample"}},
            R"([{"type":"interpolate.grid2namedgrid","interpolation":"healpix-resample","lsm-weight-adjustment":0.2,"non-linear":["missing-if-heaviest-missing"],"pole-displacement-in-degree":0,"prune-epsilon":1e-10,"solver":{"type":"multiply"},"grid":"HN2"},{"type":"save","packing":"ccsds","edition":2}])",
            R"([{"grid":"H2"},{"grid":"H2","order":"nested"},{"grid":"H2","order":"nested"}])");

        EXPECT_EQUAL(get(h, "gridType"), "healpix");
        EXPECT_EQUAL(get(h, "orderingConvention"), "nested");
        EXPECT_EQUAL(get(h, "Nside"), "2");
        EXPECT_EQUAL(get(h, "numberOfDataPoints"), "48");
        EXPECT_EQUAL(get(h, "longitudeOfFirstGridPointInDegrees"), "45");
    }

    SECTION("002") {
        // MIR-708
        // input gridType=healpix,Nside=2,orderingConvention=nested.grib2, as its grid
        auto h = run(
            "{artificialInput:gridspec,gridSpec:{grid:H2,order:nested}}",
            {{"grid", "h1"}, {"interpolation", "healpix-resample"}},
            R"([{"type":"interpolate.grid2namedgrid","interpolation":"healpix-resample","lsm-weight-adjustment":0.2,"non-linear":["missing-if-heaviest-missing"],"pole-displacement-in-degree":0,"prune-epsilon":1e-10,"solver":{"type":"multiply"},"grid":"H1"},{"type":"save","packing":"ccsds","edition":2}])",
            R"([{"grid":"H2","order":"nested"},{"grid":"H1"},{"grid":"H1"}])");

        EXPECT_EQUAL(get(h, "gridType"), "healpix");
        EXPECT_EQUAL(get(h, "orderingConvention"), "ring");
        EXPECT_EQUAL(get(h, "Nside"), "1");
        EXPECT_EQUAL(get(h, "numberOfDataPoints"), "12");
        EXPECT_EQUAL(get(h, "longitudeOfFirstGridPointInDegrees"), "45");
    }

    SECTION("003") {
        // MIR-708
        // input gridType=healpix,Nside=2,orderingConvention=nested.grib2, as its grid
        auto h = run(
            "{artificialInput:gridspec,gridSpec:{grid:H2,order:nested}}",
            {{"grid", "h4"}, {"interpolation", "healpix-resample"}},
            R"([{"type":"interpolate.grid2namedgrid","interpolation":"healpix-resample","lsm-weight-adjustment":0.2,"non-linear":["missing-if-heaviest-missing"],"pole-displacement-in-degree":0,"prune-epsilon":1e-10,"solver":{"type":"multiply"},"grid":"H4"},{"type":"save","packing":"ccsds","edition":2}])",
            R"([{"grid":"H2","order":"nested"},{"grid":"H4"},{"grid":"H4"}])");

        EXPECT_EQUAL(get(h, "gridType"), "healpix");
        EXPECT_EQUAL(get(h, "orderingConvention"), "ring");
        EXPECT_EQUAL(get(h, "Nside"), "4");
        EXPECT_EQUAL(get(h, "numberOfDataPoints"), "192");
        EXPECT_EQUAL(get(h, "longitudeOfFirstGridPointInDegrees"), "45");
    }
}


CASE("to healpix: regrid") {
    // MIR-632
    // input gridType=healpix,Nside=32,orderingConvention=ring.grib2, as its grid
    auto h = run(
        "{artificialInput:gridspec,gridSpec:{grid:H32}}", {{"grid", "H36"}},
        R"([{"type":"interpolate.grid2namedgrid","interpolation":"linear","finite-element-projection-fail":"missing-value","finite-element-validate-mesh":false,"lsm-weight-adjustment":0.2,"non-linear":["missing-if-heaviest-missing"],"pole-displacement-in-degree":0,"prune-epsilon":1e-10,"solver":{"type":"multiply"},"grid":"H36"},{"type":"save","packing":"ccsds","edition":2}])",
        R"([{"grid":"H32"},{"grid":"H36"},{"grid":"H36"}])");

    EXPECT_EQUAL(get(h, "gridType"), "healpix");
    EXPECT_EQUAL(get(h, "gridName"), "H36");
    EXPECT_EQUAL(get(h, "Nside"), "36");
    EXPECT_EQUAL(get(h, "numberOfDataPoints"), "15552");
    EXPECT_EQUAL(get(h, "longitudeOfFirstGridPointInDegrees"), "45");
}


CASE("to reduced_gg: grid-box-average") {
    // MIR-632
    // input gridType=healpix,Nside=32,orderingConvention=ring.grib2, as its grid
    auto h = run(
        "{artificialInput:gridspec,gridSpec:{grid:H32}}", {{"interpolation", "grid-box-average"}, {"grid", "O8"}},
        R"([{"type":"interpolate.grid2namedgrid","interpolation":"grid-box-average","lsm-weight-adjustment":0.2,"non-linear":["missing-if-heaviest-missing"],"pole-displacement-in-degree":0,"prune-epsilon":1e-10,"solver":{"type":"multiply"},"grid":"O8"},{"type":"save","packing":"ccsds","edition":2}])",
        R"([{"grid":"H32"},{"grid":"O8"},{"grid":"O8"}])");

    EXPECT_EQUAL(get(h, "gridType"), "reduced_gg");
    EXPECT_EQUAL(get(h, "numberOfDataPoints"), "544");
}


CASE("to reduced_gg: regrid") {
    // MIR-632
    // input gridType=healpix,Nside=32,orderingConvention=ring.grib2, as its grid
    auto h = run(
        "{artificialInput:gridspec,gridSpec:{grid:H32}}", {{"grid", "O8"}},
        R"([{"type":"interpolate.grid2namedgrid","interpolation":"linear","finite-element-projection-fail":"missing-value","finite-element-validate-mesh":false,"lsm-weight-adjustment":0.2,"non-linear":["missing-if-heaviest-missing"],"pole-displacement-in-degree":0,"prune-epsilon":1e-10,"solver":{"type":"multiply"},"grid":"O8"},{"type":"save","packing":"ccsds","edition":2}])",
        R"([{"grid":"H32"},{"grid":"O8"},{"grid":"O8"}])");

    EXPECT_EQUAL(get(h, "gridType"), "reduced_gg");
    EXPECT_EQUAL(get(h, "numberOfDataPoints"), "544");
}


CASE("to regular_gg: grid-box-average") {
    // MIR-632
    // input gridType=healpix,Nside=32,orderingConvention=ring.grib2, as its grid
    auto h = run(
        "{artificialInput:gridspec,gridSpec:{grid:H32}}", {{"interpolation", "grid-box-average"}, {"grid", "F8"}},
        R"([{"type":"interpolate.grid2namedgrid","interpolation":"grid-box-average","lsm-weight-adjustment":0.2,"non-linear":["missing-if-heaviest-missing"],"pole-displacement-in-degree":0,"prune-epsilon":1e-10,"solver":{"type":"multiply"},"grid":"F8"},{"type":"save","packing":"ccsds","edition":2}])",
        R"([{"grid":"H32"},{"grid":"F8"},{"grid":"F8"}])");

    EXPECT_EQUAL(get(h, "gridType"), "regular_gg");
    EXPECT_EQUAL(get(h, "numberOfDataPoints"), "512");
}


CASE("to regular_gg: regrid") {
    // MIR-632
    // input gridType=healpix,Nside=32,orderingConvention=ring.grib2, as its grid
    auto h = run(
        "{artificialInput:gridspec,gridSpec:{grid:H32}}", {{"grid", "F8"}},
        R"([{"type":"interpolate.grid2namedgrid","interpolation":"linear","finite-element-projection-fail":"missing-value","finite-element-validate-mesh":false,"lsm-weight-adjustment":0.2,"non-linear":["missing-if-heaviest-missing"],"pole-displacement-in-degree":0,"prune-epsilon":1e-10,"solver":{"type":"multiply"},"grid":"F8"},{"type":"save","packing":"ccsds","edition":2}])",
        R"([{"grid":"H32"},{"grid":"F8"},{"grid":"F8"}])");

    EXPECT_EQUAL(get(h, "gridType"), "regular_gg");
    EXPECT_EQUAL(get(h, "numberOfDataPoints"), "512");
}


CASE("to regular_ll: nn") {
    // MIR-684
    auto h = run(
        "{artificialInput:gridspec,gridSpec:{grid:h2,order:nested}}", {{"grid", "{grid:3/3}"}, {"interpolation", "nn"}},
        R"([{"type":"interpolate.grid2gridspec","interpolation":"nn","distance-weighting":"inverse-distance-weighting-squared","lsm-weight-adjustment":0.2,"nclosest":4,"nearest-method":"nearest-neighbour-with-lowest-index","non-linear":["missing-if-heaviest-missing"],"pole-displacement-in-degree":0,"prune-epsilon":1e-10,"solver":{"type":"multiply"},"grid":{"grid":[3,3]}},{"type":"save","packing":"ccsds","edition":2}])",
        R"([{"grid":"H2","order":"nested"},{"grid":[3,3]},{"grid":[3,3]}])");

    EXPECT_EQUAL(get(h, "numberOfDataPoints"), "7320");
}


CASE("to regular_ll: regrid") {
    // MIR-632
    // input gridType=healpix,Nside=32,orderingConvention=ring.grib2, as its grid
    auto h = run(
        "{artificialInput:gridspec,gridSpec:{grid:H32}}", {{"grid", "2/2"}},
        R"([{"type":"interpolate.grid2regular-ll","interpolation":"linear","finite-element-projection-fail":"missing-value","finite-element-validate-mesh":false,"lsm-weight-adjustment":0.2,"non-linear":["missing-if-heaviest-missing"],"pole-displacement-in-degree":0,"prune-epsilon":1e-10,"solver":{"type":"multiply"},"grid":[2,2],"area":[90,0,-90,358]},{"type":"save","packing":"ccsds","edition":2}])",
        R"([{"grid":"H32"},{"grid":[2,2]},{"grid":[2,2]}])");

    EXPECT_EQUAL(get(h, "gridType"), "regular_ll");
    EXPECT_EQUAL(get(h, "numberOfDataPoints"), "16380");
}


}  // namespace mir::tests::regressions


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
