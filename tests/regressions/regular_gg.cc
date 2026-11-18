// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "eckit/geo/eckit_geo_config.h"

#include "regression.h"


namespace mir::tests::regressions {


CASE("to healpix: grid-box-average") {
    // MIR-632
    // input gridType=regular_gg,gridName=F8,shortName=q.grib2, as its grid
    auto h = run(
        "{artificialInput:gridspec,gridSpec:{grid:F8}}", {{"interpolation", "grid-box-average"}, {"grid", "H2"}},
        R"([{"type":"interpolate.grid2namedgrid","interpolation":"grid-box-average","lsm-weight-adjustment":0.2,"non-linear":["missing-if-heaviest-missing"],"pole-displacement-in-degree":0,"prune-epsilon":1e-10,"solver":{"type":"multiply"},"grid":"H2"},{"type":"save","packing":"ccsds","edition":2}])",
        R"([{"grid":"F8"},{"grid":"H2"},{"grid":"H2"}])");

    EXPECT_EQUAL(get(h, "gridType"), "healpix");
    EXPECT_EQUAL(get(h, "gridName"), "H2");
    EXPECT_EQUAL(get(h, "Nside"), "2");
    EXPECT_EQUAL(get(h, "numberOfDataPoints"), "48");
    EXPECT_EQUAL(get(h, "longitudeOfFirstGridPointInDegrees"), "45");
}


CASE("to healpix: regrid") {
    // MIR-632
    // input gridType=regular_gg,gridName=F8,shortName=q.grib2, as its grid
    auto h = run(
        "{artificialInput:gridspec,gridSpec:{grid:F8}}", {{"grid", "H2"}},
        R"([{"type":"interpolate.grid2namedgrid","interpolation":"linear","finite-element-projection-fail":"missing-value","finite-element-validate-mesh":false,"lsm-weight-adjustment":0.2,"non-linear":["missing-if-heaviest-missing"],"pole-displacement-in-degree":0,"prune-epsilon":1e-10,"solver":{"type":"multiply"},"grid":"H2"},{"type":"save","packing":"ccsds","edition":2}])",
        R"([{"grid":"F8"},{"grid":"H2"},{"grid":"H2"}])");

    EXPECT_EQUAL(get(h, "gridType"), "healpix");
    EXPECT_EQUAL(get(h, "gridName"), "H2");
    EXPECT_EQUAL(get(h, "Nside"), "2");
    EXPECT_EQUAL(get(h, "numberOfDataPoints"), "48");
    EXPECT_EQUAL(get(h, "longitudeOfFirstGridPointInDegrees"), "45");
}


CASE("to reduced_gg: nn") {
    // input gridType=regular_gg,gridName=F32,shortName=msl.grib1, as its grid (GRIB1)
    auto h = run(
        "{artificialInput:gridspec,gridSpec:{grid:F32}}", {{"grid", "O32"}, {"interpolation", "nn"}, {"edition", "1"}},
        R"([{"type":"interpolate.grid2namedgrid","interpolation":"nn","distance-weighting":"inverse-distance-weighting-squared","lsm-weight-adjustment":0.2,"nclosest":4,"nearest-method":"nearest-neighbour-with-lowest-index","non-linear":["missing-if-heaviest-missing"],"pole-displacement-in-degree":0,"prune-epsilon":1e-10,"solver":{"type":"multiply"},"grid":"O32"},{"type":"save","packing":"simple","edition":1}])",
        R"([{"grid":"F32"},{"grid":"O32"},{"grid":"O32"}])");

    EXPECT_EQUAL(get(h, "gridName"), "O32");
}


CASE("to regular_ll: regrid") {
    // MIR-255
    // input gridType=regular_gg,gridName=F48,shortName=msl,global=0.002.grib1, as its grid (GRIB1)
    auto h = run(
        "{artificialInput:gridspec,gridSpec:{area:[90,0,-77.4058880820788,360],grid:F48}}",
        {{"grid", "1/1"}, {"edition", "1"}},
        R"([{"type":"interpolate.grid2regular-ll","interpolation":"linear","finite-element-projection-fail":"missing-value","finite-element-validate-mesh":false,"lsm-weight-adjustment":0.2,"non-linear":["missing-if-heaviest-missing"],"pole-displacement-in-degree":0,"prune-epsilon":1e-10,"solver":{"type":"multiply"},"grid":[1,1],"area":[90,0,-90,359]},{"type":"save","packing":"simple","edition":1}])",
        R"([{"area":[90,0,-77.4058880820788,360],"grid":"F48"},{"area":[90,0,-77,360],"grid":[1,1]},{"area":[90,0,-77,360],"grid":[1,1]}])");

    EXPECT_EQUAL(get(h, "Ni"), "360");
    EXPECT_EQUAL(get(h, "Nj"), "168");
    EXPECT_EQUAL(get(h, "latitudeOfFirstGridPointInDegrees"), "90");
    EXPECT_EQUAL(get(h, "longitudeOfFirstGridPointInDegrees"), "0");
    EXPECT_EQUAL(get(h, "latitudeOfLastGridPointInDegrees"), "-77");
    EXPECT_EQUAL(get(h, "longitudeOfLastGridPointInDegrees"), "359");
}


}  // namespace mir::tests::regressions


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
