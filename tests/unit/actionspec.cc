// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <memory>
#include <string>
#include <vector>

#include "eckit/testing/Test.h"

#include "mir/action/plan/Action.h"


namespace mir::tests::unit {


using A = std::unique_ptr<const action::Action>;


CASE("Action spec") {
    struct test_t {
        std::string spec;
        std::string json;
    };

    for (const auto& test : std::vector<test_t>{
             {"{type: crop.area, area: [10, -10, -10, 10]}", R"({"type":"crop.area","area":[10,-10,-10,10]})"},
             {"{type: crop.area, area: 10/-10/-10/10}", R"({"type":"crop.area","area":[10,-10,-10,10]})"},
             {"{type: mask.area, area: [60, 0, 40, 30]}", R"({"type":"mask.area","area":[60,0,40,30]})"},
             {"{type: filter.sh-bandpass, bandpass: [0, 10, 0, 20], truncation: 63}",
              R"({"type":"filter.sh-bandpass","bandpass":[0,10,0,20],"truncation":63})"},
             {"{type: filter.limiter, limiter_minimum: 0, limiter_maximum: 1}",
              R"({"type":"filter.limiter","limiter-minimum":0,"limiter-maximum":1})"},
             {"{type: filter.scalar-gradient, nabla_poles_missing_values: true}",
              R"({"type":"filter.scalar-gradient","nabla-poles-missing-values":true})"},
             {"filter.scalar-gradient", "filter.scalar-gradient"},
             {"{type: filter.scalar-laplacian}", "filter.scalar-laplacian"},
             {"filter.uv-gradient", "filter.uv-gradient"},
             {"filter.uv-divergence", "filter.uv-divergence"},
             {"filter.uv-vorticity", "filter.uv-vorticity"},
         }) {
        A a(action::ActionFactory::make_from_string(test.spec));
        EXPECT_EQUAL(a->json_str(), test.json);

        // from the (complete) spec, and from the (possibly known name) spec, the same action
        A b(action::ActionFactory::make_from_string(a->json_str(false)));
        EXPECT_EQUAL(b->json_str(false), a->json_str(false));
        EXPECT(*a == *b);

        A c(action::ActionFactory::make_from_string(a->json_str()));
        EXPECT(*a == *c);
    }
}


CASE("Action spec, by name or with options") {
    for (const std::string name : {"filter.scalar-gradient", "filter.scalar-laplacian", "filter.uv-gradient",
                                   "filter.uv-divergence", "filter.uv-vorticity"}) {
        A simple(action::ActionFactory::make_from_string(name));
        A type(action::ActionFactory::make_from_string("{type: " + name + "}"));
        A special(action::ActionFactory::make_from_string("{type: " + name + ", nabla-poles-missing-values: true}"));

        EXPECT_EQUAL(simple->json_str(), name);
        EXPECT_EQUAL(type->json_str(), name);
        EXPECT(special->json_str() != name);

        EXPECT(*simple == *type);
        EXPECT(*simple != *special);
    }
}


CASE("Action spec, comparison") {
    A crop(action::ActionFactory::make_from_string("{type: crop.area, area: [10, -10, -10, 10]}"));
    A crop_same(action::ActionFactory::make_from_string(R"({"type":"crop.area","area":"10/-10/-10/10"})"));
    A crop_other(action::ActionFactory::make_from_string("{type: crop.area, area: [10, -10, -10, 20]}"));
    A mask(action::ActionFactory::make_from_string("{type: mask.area, area: [10, -10, -10, 10]}"));

    EXPECT(*crop == *crop_same);
    EXPECT(*crop != *crop_other);
    EXPECT(*crop != *mask);

    A gradient(action::ActionFactory::make_from_string("filter.scalar-gradient"));
    A laplacian(action::ActionFactory::make_from_string("filter.scalar-laplacian"));
    EXPECT(*gradient != *laplacian);
}


CASE("Action spec, round trip") {
    // actions from their complete spec are the same (as compared)
    const std::vector<std::string> specs{
        "{type: interpolate.grid2regular-ll, grid: [1, 1]}",
        "{type: interpolate.grid2regular-ll, grid: [1, 1], area: [60, -10, 30.5, 40]}",
        "{type: interpolate.grid2regular-ll, grid: [1, 1], cropping: [60, -10, 30, 40]}",
        "{type: interpolate.grid2regular-ll, grid: [2, 2], interpolation: nn, interpolation_global_input: true}",
        "{type: interpolate.grid2regular-ll, grid: [2, 2], interpolation: linear, lsm: true}",
        "{type: interpolate.grid2regular-ll, grid: [2, 2], non_linear: missing-if-all-missing}",
        "{type: interpolate.grid2namedgrid, grid: O32}",
        "{type: interpolate.grid2octahedral-gg, octahedral: 32}",
        "{type: interpolate.grid2reduced-gg, reduced: 32}",
        "{type: interpolate.grid2regular-gg, regular: 32}",
        "{type: interpolate.grid2reduced-gg-pl-given, pl: [20, 24, 24, 20]}",
        "{type: interpolate.grid2griddef, griddef: /dev/null}",
        "{type: interpolate.grid2gridspec, grid: {grid: [1, 1], area: [60, -10, 30, 40]}}",
        "{type: transform.sh-scalar-to-gridspec, grid: {grid: O32}}",
        "{type: interpolate.grid2rotated-regular-ll, grid: [1, 1], rotation: [-40, 22]}",
        "{type: interpolate.grid2rotated-octahedral-gg, octahedral: 32, rotation: [-40, 22]}",
        "{type: interpolate.grid2rotated-namedgrid, grid: O32, rotation: [-40, 22]}",
        "{type: transform.sh-scalar-to-regular-gg, regular: 32}",
        "{type: transform.sh-scalar-to-regular-ll, grid: [1, 1], area: [60, -10, 30, 40]}",
        "{type: transform.sh-scalar-to-regular-ll, grid: [1, 1], cropping: [60, -10, 30, 40]}",
        "{type: transform.sh-vod-to-uv-octahedral-gg, octahedral: 32, atlas_trans_flt: true}",
        "{type: transform.sh-scalar-to-rotated-reduced-gg, reduced: 32, rotation: [-40, 22]}",
        "{type: transform.sh-scalar-to-namedgrid, grid: O32}",
        "{type: transform.sh-vod-to-UV}",
        "{type: filter.sh-truncate, truncation: 63}",
        "{type: filter.sh-cesaro-summation-filter, cesaro_k: 2.5, cesaro_truncation: 10}",
        "{type: filter.frame, frame: 2}",
        "{type: filter.bitmap, bitmap: /dev/null}",
        "{type: filter.adjust-winds-directions, rotation: [-40, 22]}",
        "{type: filter.adjust-winds-scale-cos-latitude}",
        "{type: filter.globalise}",
        "{type: filter.unstructured}",
        "{type: filter.mask-input-lsm, mask_input_lsm_value: 1}",
        "{type: filter.mask-output-lsm}",
        "{type: filter.statistics, statistics: scalar}",
        "{type: misc.reset-missing-values}",
        "{type: noop}",
        "{type: select.field, which: 1}",
        "{type: set.metadata, metadata: paramId=1}",
    };
    for (const std::string& spec : specs) {
        A a(action::ActionFactory::make_from_string(spec));
        A b(action::ActionFactory::make_from_string(a->json_str(false)));
        EXPECT_EQUAL(b->json_str(false), a->json_str(false));
        EXPECT(*a == *b);
    }
}


CASE("Action spec, (some) options") {
    struct test_t {
        std::string spec;
        std::string json;
    };

    const std::vector<test_t> tests{
        {"{type: interpolate.grid2regular-ll, grid: [1, 1], cropping: [60, -10, 30, 40]}",
         R"({"type":"interpolate.grid2regular-ll","interpolation":"linear","finite-element-projection-fail":"missing-value",)"
         R"("finite-element-validate-mesh":false,"lsm-weight-adjustment":0.2,"non-linear":["missing-if-heaviest-missing"],)"
         R"("pole-displacement-in-degree":0,"prune-epsilon":1e-10,"solver":{"type":"multiply"},"cropping":[60,-10,30,40],)"
         R"("grid":[1,1],"area":[90,0,-90,359]})"},
        {"{type: interpolate.grid2namedgrid, grid: O32, interpolation: grid-box-statistics, "
         "interpolation_statistics: "
         "maximum}",
         R"({"type":"interpolate.grid2namedgrid","interpolation":"grid-box-statistics","interpolation-statistics":)"
         R"("maximum","lsm-weight-adjustment":0.2,"non-linear":["missing-if-heaviest-missing"],)"
         R"("pole-displacement-in-degree":0,"prune-epsilon":1e-10,"solver":{"type":"statistics","statistics":)"
         R"({"type":"maximum"}},"grid":"O32"})"},
        {"{type: interpolate.grid2namedgrid, grid: O32, interpolation: k-nearest, nearest_method: distance, "
         "distance_weighting: cressman, distance: 1000}",
         R"({"type":"interpolate.grid2namedgrid","interpolation":"k-nearest","distance":1000,"distance-weighting":)"
         R"("cressman","lsm-weight-adjustment":0.2,"nearest-method":"distance","non-linear":)"
         R"(["missing-if-heaviest-missing"],"pole-displacement-in-degree":0,"prune-epsilon":1e-10,"solver":)"
         R"({"type":"multiply"},"grid":"O32"})"},
        {"{type: transform.sh-scalar-to-regular-gg, regular: 32, atlas_trans_flt: true}",
         R"({"type":"transform.sh-scalar-to-regular-gg","atlas-trans-flt":true,"atlas-trans-type":"local","invtrans":"scalar",)"
         R"("regular":32})"},
        {"{type: transform.sh-vod-to-UV}", R"({"type":"transform.sh-vod-to-UV","atlas-trans-type":"local"})"},
        {"{type: filter.mask-input-lsm, mask_input_lsm_value: 1}",
         R"({"type":"filter.mask-input-lsm","mask-input-lsm-value":1})"},
    };
    for (const auto& test : tests) {
        A a(action::ActionFactory::make_from_string(test.spec));
        EXPECT_EQUAL(a->json_str(), test.json);
    }
}


CASE("Action spec, errors") {
    EXPECT_THROWS(A(action::ActionFactory::make_from_string("")));
    EXPECT_THROWS(A(action::ActionFactory::make_from_string("{area: [10, -10, -10, 10]}")));
    EXPECT_THROWS(A(action::ActionFactory::make_from_string("{type: unknown}")));
}


}  // namespace mir::tests::unit


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
