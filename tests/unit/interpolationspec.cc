/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 *
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include <memory>
#include <string>
#include <vector>

#include "eckit/testing/Test.h"

#include "mir/method/Method.h"
#include "mir/param/CombinedParametrisation.h"


namespace mir::tests::unit {


static const auto L{
    R"({"type":"linear","non-linear":["missing-if-heaviest-missing"],"finite-element-validate-mesh":false,"finite-element-projection-fail":"missing-value"})"};
static const auto B{
    R"({"type":"bilinear","non-linear":["missing-if-heaviest-missing"],"finite-element-validate-mesh":false,"finite-element-projection-fail":"missing-value"})"};
static const auto NN{
    R"({"type":"nearest-neighbour","non-linear":["missing-if-heaviest-missing"],"nearest-method":"nearest-neighbour-with-lowest-index","distance-weighting":"inverse-distance-weighting-squared"})"};
static const auto NN5{
    R"({"type":"nearest-neighbour","non-linear":["missing-if-heaviest-missing"],"nearest-method":"nearest-neighbour-with-lowest-index","nclosest":5,"distance-weighting":"inverse-distance-weighting-squared"})"};
static const auto GBA{R"({"type":"grid-box-average","non-linear":["missing-if-heaviest-missing"]})"};
static const auto GBMAX{
    R"({"type":"grid-box-statistics","non-linear":["missing-if-heaviest-missing"],"interpolation-statistics":"maximum"})"};
static const auto GBMIN{
    R"({"type":"grid-box-statistics","non-linear":["missing-if-heaviest-missing"],"interpolation-statistics":"minimum"})"};
static const auto VMAX{
    R"({"type":"voronoi-statistics","non-linear":["missing-if-heaviest-missing"],"interpolation-statistics":"maximum"})"};
static const auto VMIN{
    R"({"type":"voronoi-statistics","non-linear":["missing-if-heaviest-missing"],"interpolation-statistics":"minimum"})"};


CASE("interpolationSpec") {
    struct test_t {
        std::string spec_reference;
        std::string spec_extended;
        std::string spec;
    };


    SECTION("Method from name") {
        param::SimpleParametrisation empty;
        param::CombinedParametrisation param(empty);

        for (const auto& test : std::vector<test_t>{
                 {"linear", L, "linear"},
                 {"nearest-neighbour", NN, "nearest-neighbour"},
                 {"nearest-neighbour", NN, "nearest-neighbor"},
                 {"nearest-neighbour", NN, "nn"},
                 {"bilinear", B, "bilinear"},
                 {"grid-box-average", GBA, "grid-box-average"},
             }) {
            std::unique_ptr<const method::Method> a(method::MethodFactory::build(test.spec, param));

            EXPECT(a->json_str() == test.spec_reference);

            std::unique_ptr<const method::Method> b(method::MethodFactory::make_from_string(test.spec));

            EXPECT(b->json_str() == test.spec_reference);
            EXPECT(b->json_str(false) == test.spec_extended);
        }
    }


    SECTION("Method from spec") {
        for (const auto& test : std::vector<test_t>{
                 {"linear", L, ""},
                 {"linear", L, "linear"},
                 {"linear", L, "{type: linear}"},

                 {"nearest-neighbour", NN, "nearest-neighbour"},
                 {"nearest-neighbour", NN, "nn"},
                 {"nearest-neighbour", NN, "{type: nearest-neighbor, dummy: 4}"},
                 {"nearest-neighbour", NN, "{type: nearest-neighbor, nclosest: 4}"},

                 {NN5, NN5, "{type: nn, nclosest: 5}"},

                 {"grid-box-average", GBA, "grid-box-average"},
                 {"grid-box-average", GBA, "{type: grid-box-average, dummy: -1}"},

                 {GBMAX, GBMAX, "grid-box-statistics"},
                 {GBMAX, GBMAX, "{type: grid-box-statistics, dummy: -1}"},

                 {GBMIN, GBMIN, "{type: grid-box-statistics, interpolation-statistics: minimum}"},
                 {GBMIN, GBMIN, "{type: grid-box-statistics, interpolation_statistics: minimum, dummy: -1}"},

                 {VMAX, VMAX, "voronoi-statistics"},
                 {VMAX, VMAX, "{type: voronoi-statistics, dummy: -1}"},

                 {VMIN, VMIN, "{type: voronoi-statistics, interpolation-statistics: minimum}"},
                 {VMIN, VMIN, "{type: voronoi-statistics, interpolation_statistics: minimum, dummy: -1}"},
             }) {
            std::unique_ptr<const method::Method> m(method::MethodFactory::make_from_string(test.spec));

            EXPECT(m->json_str() == test.spec_reference);
            EXPECT(m->json_str(false) == test.spec_extended);
        }
    }
}


}  // namespace mir::tests::unit


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
