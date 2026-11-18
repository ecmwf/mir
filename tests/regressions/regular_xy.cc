// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "eckit/geo/eckit_geo_config.h"

#include "regression.h"


namespace mir::tests::regressions {


#if eckit_HAVE_PROJ
CASE("to reduced_gg: linear") {
    auto h = run(
        "{artificialInput:gridspec,gridSpec:{type:swisslv95,x:[2480000,2840000,20000],y:[1080000,1300000,20000]}}",
        {{"grid", "{grid:o320}"}, {"interpolation", "linear"}},
        R"([{"type":"interpolate.grid2gridspec","interpolation":"linear","finite-element-projection-fail":"missing-value","finite-element-validate-mesh":false,"lsm-weight-adjustment":0.2,"non-linear":["missing-if-heaviest-missing"],"pole-displacement-in-degree":0,"prune-epsilon":1e-10,"solver":{"type":"multiply"},"grid":{"grid":"O320"}},{"type":"save","packing":"ccsds","edition":2}])",
        R"([{"order":"i+j+","projection":"swisslv95","x":[2480000,2840000,20000],"y":[1080000,1300000,20000]},{"area":[47.6346412794268,5.83564470245482,45.9484597060362,10.6432135221386],"grid":"O320"},{"area":[47.6346412794268,5.83564470245482,45.9484597060362,10.6432135221386],"grid":"O320"}])");

    EXPECT_EQUAL(get(h, "edition"), "2");
    EXPECT_EQUAL(get(h, "gridType"), "reduced_gg");
    EXPECT_EQUAL(get(h, "numberOfDataPoints"), "57");
    EXPECT_EQUAL(get(h, "bitmapPresent"), "1");
    EXPECT_EQUAL(get(h, "numberOfMissing"), "1");
}
#endif


#if eckit_HAVE_PROJ
CASE("to reduced_gg: nn") {
    auto h = run(
        "{artificialInput:gridspec,gridSpec:{type:swisslv95,x:[2480000,2840000,20000],y:[1080000,1300000,20000]}}",
        {{"grid", "{grid:o320}"}, {"interpolation", "nn"}},
        R"([{"type":"interpolate.grid2gridspec","interpolation":"nn","distance-weighting":"inverse-distance-weighting-squared","lsm-weight-adjustment":0.2,"nclosest":4,"nearest-method":"nearest-neighbour-with-lowest-index","non-linear":["missing-if-heaviest-missing"],"pole-displacement-in-degree":0,"prune-epsilon":1e-10,"solver":{"type":"multiply"},"grid":{"grid":"O320"}},{"type":"save","packing":"ccsds","edition":2}])",
        R"([{"order":"i+j+","projection":"swisslv95","x":[2480000,2840000,20000],"y":[1080000,1300000,20000]},{"area":[47.6346412794268,5.83564470245482,45.9484597060362,10.6432135221386],"grid":"O320"},{"area":[47.6346412794268,5.83564470245482,45.9484597060362,10.6432135221386],"grid":"O320"}])");

    EXPECT_EQUAL(get(h, "gridType"), "reduced_gg");
    EXPECT_EQUAL(get(h, "N"), "320");
    EXPECT_EQUAL(get(h, "numberOfDataPoints"), "57");
    EXPECT_EQUAL(get(h, "latitudeOfFirstGridPointInDegrees"), "47.8505");
    EXPECT_EQUAL(get(h, "longitudeOfFirstGridPointInDegrees"), "5.83564");
    EXPECT_EQUAL(get(h, "latitudeOfLastGridPointInDegrees"), "45.8291");
    EXPECT_EQUAL(get(h, "longitudeOfLastGridPointInDegrees"), "10.6432");
}
#endif


#if eckit_HAVE_PROJ
CASE("to regular_ll: grid-box-statistics") {
    auto h = run(
        "{artificialInput:gridspec,gridSpec:{type:swisslv95,x:[2480000,2840000,20000],y:[1080000,1300000,20000]}}",
        {{"grid", "{grid:0.25/0.25}"}, {"interpolation", "grid-box-statistics"}},
        R"([{"type":"interpolate.grid2gridspec","interpolation":"grid-box-statistics","interpolation-statistics":"maximum","lsm-weight-adjustment":0.2,"non-linear":["missing-if-heaviest-missing"],"pole-displacement-in-degree":0,"prune-epsilon":1e-10,"solver":{"type":"statistics","statistics":{"type":"maximum"}},"grid":{"grid":[0.25,0.25]}},{"type":"save","packing":"ccsds","edition":2}])",
        R"([{"order":"i+j+","projection":"swisslv95","x":[2480000,2840000,20000],"y":[1080000,1300000,20000]},{"area":[47.75,6,46,10.5],"grid":[0.25,0.25]},{"area":[47.75,6,46,10.5],"grid":[0.25,0.25]}])");

    EXPECT_EQUAL(get(h, "edition"), "2");
    EXPECT_EQUAL(get(h, "gridType"), "regular_ll");
    EXPECT_EQUAL(get(h, "numberOfDataPoints"), "152");
    EXPECT_EQUAL(get(h, "bitmapPresent"), "1");
    EXPECT_EQUAL(get(h, "numberOfMissing"), "15");
}
#endif


#if eckit_HAVE_PROJ
CASE("to regular_ll: nn") {
    auto h = run(
        "{artificialInput:gridspec,gridSpec:{type:swisslv95,x:[2480000,2840000,20000],y:[1080000,1300000,20000]}}",
        {{"grid", "{grid:0.25/0.25}"}, {"interpolation", "nn"}},
        R"([{"type":"interpolate.grid2gridspec","interpolation":"nn","distance-weighting":"inverse-distance-weighting-squared","lsm-weight-adjustment":0.2,"nclosest":4,"nearest-method":"nearest-neighbour-with-lowest-index","non-linear":["missing-if-heaviest-missing"],"pole-displacement-in-degree":0,"prune-epsilon":1e-10,"solver":{"type":"multiply"},"grid":{"grid":[0.25,0.25]}},{"type":"save","packing":"ccsds","edition":2}])",
        R"([{"order":"i+j+","projection":"swisslv95","x":[2480000,2840000,20000],"y":[1080000,1300000,20000]},{"area":[47.75,6,46,10.5],"grid":[0.25,0.25]},{"area":[47.75,6,46,10.5],"grid":[0.25,0.25]}])");

    EXPECT_EQUAL(get(h, "gridType"), "regular_ll");
    EXPECT_EQUAL(get(h, "Ni"), "19");
    EXPECT_EQUAL(get(h, "Nj"), "8");
    EXPECT_EQUAL(get(h, "numberOfDataPoints"), "152");
    EXPECT_EQUAL(get(h, "latitudeOfFirstGridPointInDegrees"), "47.75");
    EXPECT_EQUAL(get(h, "longitudeOfFirstGridPointInDegrees"), "6");
    EXPECT_EQUAL(get(h, "latitudeOfLastGridPointInDegrees"), "46");
    EXPECT_EQUAL(get(h, "longitudeOfLastGridPointInDegrees"), "10.5");
    EXPECT_EQUAL(get(h, "numberOfMissing"), "0");
}
#endif


#if eckit_HAVE_PROJ
CASE("to rotated_ll: nn") {
    auto h = run(
        "{artificialInput:gridspec,gridSpec:{type:swisslv95,x:[2480000,2840000,20000],y:[1080000,1300000,20000]}}",
        {{"grid", "{grid:0.25/0.25,area:[0.75,-2.5,-1,0.25],rotation:[-43,10]}"}, {"interpolation", "nn"}},
        R"([{"type":"interpolate.grid2gridspec","interpolation":"nn","distance-weighting":"inverse-distance-weighting-squared","lsm-weight-adjustment":0.2,"nclosest":4,"nearest-method":"nearest-neighbour-with-lowest-index","non-linear":["missing-if-heaviest-missing"],"pole-displacement-in-degree":0,"prune-epsilon":1e-10,"solver":{"type":"multiply"},"grid":{"area":[0.75,-2.5,-1,0.25],"grid":[0.25,0.25],"projection":{"south_pole":[10,-43],"type":"rotation"}}},{"type":"save","packing":"ccsds","edition":2}])",
        R"([{"order":"i+j+","projection":"swisslv95","x":[2480000,2840000,20000],"y":[1080000,1300000,20000]},{"area":[0.75,-2.5,-1,0.25],"grid":[0.25,0.25],"projection":{"south_pole":[10,-43],"type":"rotation"}},{"area":[0.75,-2.5,-1,0.25],"grid":[0.25,0.25],"projection":{"south_pole":[10,-43],"type":"rotation"}}])");

    EXPECT_EQUAL(get(h, "gridType"), "rotated_ll");
    EXPECT_EQUAL(get(h, "Ni"), "12");
    EXPECT_EQUAL(get(h, "Nj"), "8");
    EXPECT_EQUAL(get(h, "numberOfDataPoints"), "96");
    EXPECT_EQUAL(get(h, "latitudeOfSouthernPoleInDegrees"), "-43");
    EXPECT_EQUAL(get(h, "longitudeOfSouthernPoleInDegrees"), "10");
    EXPECT_EQUAL(get(h, "numberOfMissing"), "0");
}
#endif


#if eckit_HAVE_PROJ
CASE("to unstructured_grid: nn") {
    auto h = run(
        "{artificialInput:gridspec,gridSpec:{type:swisslv95,x:[2480000,2840000,20000],y:[1080000,1300000,20000]}}",
        {{"grid", "{grid:ICON-CH2}"}, {"interpolation", "nn"}},
        R"([{"type":"interpolate.grid2gridspec","interpolation":"nn","distance-weighting":"inverse-distance-weighting-squared","lsm-weight-adjustment":0.2,"nclosest":4,"nearest-method":"nearest-neighbour-with-lowest-index","non-linear":["missing-if-heaviest-missing"],"pole-displacement-in-degree":0,"prune-epsilon":1e-10,"solver":{"type":"multiply"},"grid":{"area":[50.6,-0.9,41.9,17.8],"grid":"ICON-CH2_C","uid":"bbbd5a09855499243c7a4aa4c8762920"}},{"type":"save","packing":"ccsds","edition":2}])",
        R"([{"order":"i+j+","projection":"swisslv95","x":[2480000,2840000,20000],"y":[1080000,1300000,20000]},{"area":[50.6,-0.9,41.9,17.8],"grid":"ICON-CH2_C","uid":"bbbd5a09855499243c7a4aa4c8762920"},{"area":[50.6,-0.9,41.9,17.8],"grid":"ICON-CH2_C","uid":"bbbd5a09855499243c7a4aa4c8762920"}])");

    EXPECT_EQUAL(get(h, "gridType"), "unstructured_grid");
    EXPECT_EQUAL(get(h, "numberOfDataPoints"), "283876");
    EXPECT_EQUAL(get(h, "uuidOfHGrid"), "bbbd5a09855499243c7a4aa4c8762920");
    EXPECT_EQUAL(get(h, "numberOfMissing"), "263962");
}
#endif


}  // namespace mir::tests::regressions


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
