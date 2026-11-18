// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "eckit/geo/eckit_geo_config.h"

#include "regression.h"


namespace mir::tests::regressions {


CASE("to rotated_ll: nn") {
    auto h = run(
        "{artificialInput:gridspec,gridSpec:{grid:ICON-CH2}}",
        {{"grid", "{grid:0.5/0.5,area:[2,-4,-3,4],rotation:[-43,10]}"}, {"interpolation", "nn"}},
        R"([{"type":"interpolate.grid2gridspec","interpolation":"nn","distance-weighting":"inverse-distance-weighting-squared","lsm-weight-adjustment":0.2,"nclosest":4,"nearest-method":"nearest-neighbour-with-lowest-index","non-linear":["missing-if-heaviest-missing"],"pole-displacement-in-degree":0,"prune-epsilon":1e-10,"solver":{"type":"multiply"},"grid":{"area":[2,-4,-3,4],"grid":[0.5,0.5],"projection":{"south_pole":[10,-43],"type":"rotation"}}},{"type":"save","packing":"ccsds","edition":2}])",
        R"([{"area":[50.6,-0.9,41.9,17.8],"grid":"ICON-CH2_C","uid":"bbbd5a09855499243c7a4aa4c8762920"},{"area":[2,-4,-3,4],"grid":[0.5,0.5],"projection":{"south_pole":[10,-43],"type":"rotation"}},{"area":[2,-4,-3,4],"grid":[0.5,0.5],"projection":{"south_pole":[10,-43],"type":"rotation"}}])");

    EXPECT_EQUAL(get(h, "gridType"), "rotated_ll");
    EXPECT_EQUAL(get(h, "Ni"), "17");
    EXPECT_EQUAL(get(h, "Nj"), "11");
    EXPECT_EQUAL(get(h, "numberOfDataPoints"), "187");
    EXPECT_EQUAL(get(h, "latitudeOfSouthernPoleInDegrees"), "-43");
    EXPECT_EQUAL(get(h, "longitudeOfSouthernPoleInDegrees"), "10");
    EXPECT_EQUAL(get(h, "numberOfMissing"), "0");
}


CASE("to unstructured_grid: random") {
    // MIR-679
    auto h = run(
        "{artificialInput:gridspec,gridSpec:" + unstructured("../data/weather-params.pts") + "}",
        {{"add-random", "uniform-int-distribution{a:1,b:3}"},
         {"grid", unstructured("../data/ICON.pts")},
         {"interpolation", "nn"}},
        R"([{"type":"interpolate.grid2gridspec","interpolation":"nn","distance-weighting":"inverse-distance-weighting-squared","lsm-weight-adjustment":0.2,"nclosest":4,"nearest-method":"nearest-neighbour-with-lowest-index","non-linear":["missing-if-heaviest-missing"],"pole-displacement-in-degree":0,"prune-epsilon":1e-10,"solver":{"type":"multiply"},"grid":{"type":"unstructured_ll","uid":"9e7802ba96d2d647b36fa216bf0e28fd"}},{"type":"filter.add-random","add-random":"uniform-int-distribution{a:1,b:3}"},{"type":"save","packing":"ccsds","edition":2}])",
        R"([{"type":"unstructured_ll","uid":"68e1e3f684c3d81978edc929aa6473e9"},{"type":"unstructured_ll","uid":"9e7802ba96d2d647b36fa216bf0e28fd"},{"type":"unstructured_ll","uid":"9e7802ba96d2d647b36fa216bf0e28fd"},{"type":"unstructured_ll","uid":"9e7802ba96d2d647b36fa216bf0e28fd"}])");

    EXPECT_EQUAL(get(h, "gridType"), "unstructured_grid");
    EXPECT_EQUAL(get(h, "numberOfDataPoints"), "20480");
}


}  // namespace mir::tests::regressions


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
