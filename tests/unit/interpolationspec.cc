// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <memory>
#include <string>
#include <utility>
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


    SECTION("Method spec, round trip") {
        for (const std::string name :
             {"linear", "bilinear", "nearest-neighbour", "k-nearest", "grid-box-average", "grid-box-statistics",
              "voronoi-statistics", "healpix-resample", "healpix-statistics", "structured-bilinear",
              "structured-bicubic", "structured-biquasicubic"}) {
            std::unique_ptr<const method::Method> a(method::MethodFactory::make_from_string(name));

            // from the (complete) spec, the same method
            const auto spec = a->json_str(false);
            std::unique_ptr<const method::Method> b(method::MethodFactory::make_from_string(spec));
            EXPECT_EQUAL(b->json_str(false), spec);
            EXPECT(*a == *b);

            // from the (possibly known name) spec, the same method
            std::unique_ptr<const method::Method> c(method::MethodFactory::make_from_string(a->json_str()));
            EXPECT_EQUAL(c->json_str(), a->json_str());
            EXPECT(*a == *c);
        }
    }


    SECTION("Method spec, by name or with options") {
        using M = std::unique_ptr<const method::Method>;
        for (const auto& [name, options] : std::vector<std::pair<std::string, std::string>>{
                 {"linear", "finite-element-validate-mesh: true"},
                 {"bilinear", "finite_element_validate_mesh: true"},
                 {"nearest-neighbour", "nclosest: 5"},
                 {"k-nearest", "nclosest: 8"},
                 {"grid-box-average", "non-linear: [missing-if-all-missing]"},
                 {"grid-box-statistics", "interpolation-statistics: minimum"},
                 {"voronoi-statistics", "interpolation-statistics: mean"},
                 {"healpix-resample", "non-linear: [missing-if-all-missing]"},
                 {"healpix-statistics", "interpolation-statistics: minimum"},
                 {"structured-bilinear", "interpolation-matrix-free: false"},
                 {"structured-bicubic", "interpolation-matrix-free: false"},
                 {"structured-biquasicubic", "interpolation-matrix-free: false"},
             }) {
            M simple(method::MethodFactory::make_from_string(name));
            M type(method::MethodFactory::make_from_string("{type: " + name + "}"));
            M special(method::MethodFactory::make_from_string("{type: " + name + ", " + options + "}"));

            EXPECT(*simple == *type);
            EXPECT(*simple != *special);

            M same(method::MethodFactory::make_from_string(special->json_str()));
            EXPECT(*special == *same);
        }
    }


    SECTION("Method spec, known names") {
        for (const auto& [spec, known] : std::vector<std::pair<std::string, std::string>>{
                 {L, "linear"},
                 {"{type: structured-bilinear}", "structured-bilinear"},
                 {"{type: structured-bicubic, interpolation_matrix_free: true}", "structured-bicubic"},
                 {"{type: structured-biquasicubic, interpolation-matrix-free: true}", "structured-biquasicubic"},
             }) {
            std::unique_ptr<const method::Method> m(method::MethodFactory::make_from_string(spec));
            EXPECT_EQUAL(m->json_str(), known);
        }

        // not known (a different option)
        std::unique_ptr<const method::Method> m(
            method::MethodFactory::make_from_string("{type: structured-bilinear, interpolation-matrix-free: false}"));
        EXPECT_EQUAL(m->json_str(), R"({"type":"structured-bilinear","interpolation-matrix-free":false})");

        std::unique_ptr<const method::Method> linear(method::MethodFactory::make_from_string("linear"));
        std::unique_ptr<const method::Method> bilinear(method::MethodFactory::make_from_string("bilinear"));
        EXPECT(*linear != *bilinear);
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

                 {"grid-box-statistics", GBMAX, "grid-box-statistics"},
                 {"grid-box-statistics", GBMAX, "{type: grid-box-statistics, dummy: -1}"},

                 {GBMIN, GBMIN, "{type: grid-box-statistics, interpolation-statistics: minimum}"},
                 {GBMIN, GBMIN, "{type: grid-box-statistics, interpolation_statistics: minimum, dummy: -1}"},

                 {"voronoi-statistics", VMAX, "voronoi-statistics"},
                 {"voronoi-statistics", VMAX, "{type: voronoi-statistics, dummy: -1}"},

                 {VMIN, VMIN, "{type: voronoi-statistics, interpolation-statistics: minimum}"},
                 {VMIN, VMIN, "{type: voronoi-statistics, interpolation_statistics: minimum, dummy: -1}"},
             }) {
            std::unique_ptr<const method::Method> m(method::MethodFactory::make_from_string(test.spec));

            EXPECT(m->json_str() == test.spec_reference);
            EXPECT(m->json_str(false) == test.spec_extended);
        }
    }
}


CASE("Interpolation spec, by example (linear, k-nn)") {
    using M   = std::unique_ptr<const method::Method>;
    auto make = [](const std::string& spec) { return M(method::MethodFactory::make_from_string(spec)); };

    struct test_t {
        std::string type;
        std::string options;              // non-default
        std::string options_underscores;  // the same, with underscores in the keys
    };

    for (const auto& test : std::vector<test_t>{
             {"linear", "finite-element-validate-mesh: true", "finite_element_validate_mesh: true"},
             {"k-nearest", "nclosest: 8, nearest-method: distance-and-nclosest, distance: 100000",
              "nclosest: 8, nearest_method: distance-and-nclosest, distance: 100000"},
         }) {
        // bare name: the method with its defaults, its spec is the name
        auto simple = make(test.type);
        EXPECT_EQUAL(simple->json_str(), test.type);

        // {type: name}: exactly the same method
        EXPECT(*make("{type: " + test.type + "}") == *simple);

        // with options: a different method, its spec lists every non-default option, and rebuilds an equal method
        auto special = make("{type: " + test.type + ", " + test.options + "}");
        EXPECT(*special != *simple);
        EXPECT(special->json_str() != test.type);
        EXPECT(*make(special->json_str()) == *special);

        // option keys: underscores as hyphens
        EXPECT(*make("{type: " + test.type + ", " + test.options_underscores + "}") == *special);
    }
}


}  // namespace mir::tests::unit


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
