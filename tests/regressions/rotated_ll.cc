// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "eckit/geo/eckit_geo_config.h"

#include "regression.h"


namespace mir::tests::regressions {


CASE("to reduced_gg: nn") {
    // input rotated_ll.3-3.grib1, as its grid (GRIB1)
    auto h = run(
        "{artificialInput:gridspec,gridSpec:{area:[69,-60,21,60],grid:[3,3],projection:{south_pole:[30,30],type:"
        "rotation}}}",
        {{"grid", "{grid:o32}"}, {"interpolation", "nn"}, {"edition", "1"}},
        R"([{"type":"interpolate.grid2gridspec","interpolation":"nn","distance-weighting":"inverse-distance-weighting-squared","lsm-weight-adjustment":0.2,"nclosest":4,"nearest-method":"nearest-neighbour-with-lowest-index","non-linear":["missing-if-heaviest-missing"],"pole-displacement-in-degree":0,"prune-epsilon":1e-10,"solver":{"type":"multiply"},"grid":{"grid":"O32"}},{"type":"save","packing":"simple","edition":1}])",
        R"([{"area":[69,-60,21,60],"grid":[3,3],"projection":{"south_pole":[30,30],"type":"rotation"}},{"area":[37.6730896290453,-206.077636930463,-18.1389709902393,-93.9223794245295],"grid":"O32"},{"area":[37.6730896290453,-206.077636930463,-18.1389709902393,-93.9223794245295],"grid":"O32"}])");

    EXPECT_EQUAL(get(h, "gridType"), "reduced_gg");
    EXPECT_EQUAL(get(h, "N"), "32");
    EXPECT_EQUAL(get(h, "numberOfDataPoints"), "798");
    EXPECT_EQUAL(get(h, "latitudeOfFirstGridPointInDegrees"), "37.674");
    EXPECT_EQUAL(get(h, "longitudeOfFirstGridPointInDegrees"), "-205.715");
    EXPECT_EQUAL(get(h, "latitudeOfLastGridPointInDegrees"), "-18.139");
    EXPECT_EQUAL(get(h, "longitudeOfLastGridPointInDegrees"), "-95");
}


CASE("to regular_ll: nn") {
    // input rotated_ll.3-3.grib1, as its grid (GRIB1)
    auto h = run(
        "{artificialInput:gridspec,gridSpec:{area:[69,-60,21,60],grid:[3,3],projection:{south_pole:[30,30],type:"
        "rotation}}}",
        {{"grid", "{grid:5/5}"}, {"interpolation", "nn"}, {"edition", "1"}},
        R"([{"type":"interpolate.grid2gridspec","interpolation":"nn","distance-weighting":"inverse-distance-weighting-squared","lsm-weight-adjustment":0.2,"nclosest":4,"nearest-method":"nearest-neighbour-with-lowest-index","non-linear":["missing-if-heaviest-missing"],"pole-displacement-in-degree":0,"prune-epsilon":1e-10,"solver":{"type":"multiply"},"grid":{"grid":[5,5]}},{"type":"save","packing":"simple","edition":1}])",
        R"([{"area":[69,-60,21,60],"grid":[3,3],"projection":{"south_pole":[30,30],"type":"rotation"}},{"area":[35,-205,-15,-95],"grid":[5,5]},{"area":[35,-205,-15,-95],"grid":[5,5]}])");

    EXPECT_EQUAL(get(h, "gridType"), "regular_ll");
    EXPECT_EQUAL(get(h, "Ni"), "23");
    EXPECT_EQUAL(get(h, "Nj"), "11");
    EXPECT_EQUAL(get(h, "numberOfDataPoints"), "253");
    EXPECT_EQUAL(get(h, "latitudeOfFirstGridPointInDegrees"), "35");
    EXPECT_EQUAL(get(h, "longitudeOfFirstGridPointInDegrees"), "-205");
    EXPECT_EQUAL(get(h, "latitudeOfLastGridPointInDegrees"), "-15");
    EXPECT_EQUAL(get(h, "longitudeOfLastGridPointInDegrees"), "-95");
}


CASE("to unstructured_grid: nn") {
    auto h = run(
        "{artificialInput:gridspec,gridSpec:{grid:1/1,area:[6,-12,-6,12],rotation:[-43,10]}}",
        {{"grid", "{grid:ICON-CH2}"}, {"interpolation", "nn"}},
        R"([{"type":"interpolate.grid2gridspec","interpolation":"nn","distance-weighting":"inverse-distance-weighting-squared","lsm-weight-adjustment":0.2,"nclosest":4,"nearest-method":"nearest-neighbour-with-lowest-index","non-linear":["missing-if-heaviest-missing"],"pole-displacement-in-degree":0,"prune-epsilon":1e-10,"solver":{"type":"multiply"},"grid":{"area":[50.6,-0.9,41.9,17.8],"grid":"ICON-CH2"}},{"type":"save","packing":"ccsds","edition":2}])",
        R"([{"area":[6,-12,-6,12],"grid":[1,1],"projection":{"south_pole":[10,-43],"type":"rotation"}},{"area":[50.6,-0.9,41.9,17.8],"grid":"ICON-CH2"},{"area":[50.6,-0.9,41.9,17.8],"grid":"ICON-CH2"}])");

    EXPECT_EQUAL(get(h, "gridType"), "unstructured_grid");
    EXPECT_EQUAL(get(h, "numberOfDataPoints"), "283876");
    EXPECT_EQUAL(get(h, "uuidOfHGrid"), "bbbd5a09855499243c7a4aa4c8762920");
    EXPECT_EQUAL(get(h, "numberOfMissing"), "0");
}


}  // namespace mir::tests::regressions


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
