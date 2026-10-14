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


CASE("Action spec, errors") {
    EXPECT_THROWS(A(action::ActionFactory::make_from_string("")));
    EXPECT_THROWS(A(action::ActionFactory::make_from_string("{area: [10, -10, -10, 10]}")));
    EXPECT_THROWS(A(action::ActionFactory::make_from_string("{type: unknown}")));
}


}  // namespace mir::tests::unit


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
