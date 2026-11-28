// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "eckit/testing/Test.h"
#include "eckit/utils/MD5.h"

#include "mir/param/CombinedParametrisation.h"
#include "mir/param/DefaultParametrisation.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/util/BoundingBox.h"

#include "interpolation.h"


namespace mir::interpolation::tests {


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
        for (const auto& name : std::vector<std::string>{
                 "linear", "bilinear", "nearest-neighbour", "k-nearest", "grid-box-average", "grid-box-statistics",
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
        struct test_t {
            std::string name;
            std::string options;
        };

        for (const auto& test : std::vector<test_t>{
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
            auto simple  = make(test.name);
            auto type    = make("{type: " + test.name + "}");
            auto special = make("{type: " + test.name + ", " + test.options + "}");

            EXPECT(*simple == *type);
            EXPECT(*simple != *special);

            auto same = make(special->json_str());
            EXPECT(*special == *same);
        }
    }


    SECTION("Method spec, known names") {
        struct test_t {
            std::string spec;
            std::string known;
        };

        for (const auto& test : std::vector<test_t>{
                 {L, "linear"},
                 {"{type: structured-bilinear}", "structured-bilinear"},
                 {"{type: structured-bicubic, interpolation_matrix_free: true}", "structured-bicubic"},
                 {"{type: structured-biquasicubic, interpolation-matrix-free: true}", "structured-biquasicubic"},
             }) {
            EXPECT_EQUAL(make(test.spec)->json_str(), test.known);
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
    struct test_t {
        std::string type;
        std::string options;              // non-default
        std::string options_underscores;  // the same, with underscores in the keys
    };

    for (const auto& test : std::vector<test_t>{
             {"linear", "finite-element-validate-mesh: true", "finite_element_validate_mesh: true"},
             {"k-nearest", "nclosest: 8, nearest-method: distance-and-nclosest, distance: 100000",
              "nclosest: 8, nearest_method: distance-and-nclosest, distance: 100000"},
             {"k-nearest", "distance-weighting: cressman, distance: 1000, cressman-model-extension-power: 2",
              "distance_weighting: cressman, distance: 1000, cressman_model_extension_power: 2"},
             {"k-nearest", "nearest-method: distance, distance-weighting: cressman, distance: 1000",
              "nearest_method: distance, distance_weighting: cressman, distance: 1000"},
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


CASE("MethodFactory") {
    SECTION("list") {
        std::ostringstream list;
        method::MethodFactory::list(list);

        for (const auto& name : std::vector<std::string>{"linear", "nn", "k-nearest", "grid-box-average", "fail",
                                                         "matrix", "restructure", "voronoi-statistics"}) {
            EXPECT(list.str().find(name) != std::string::npos);
        }
    }


    SECTION("aliases") {
        struct test_t {
            std::string name;
            std::string alias;
        };

        for (const auto& test : std::vector<test_t>{
                 {"nearest-neighbour", "nearest-neighbor"},
                 {"nearest-neighbour", "nn"},
                 {"k-nearest", "k-nearest-neighbours"},
                 {"k-nearest", "k-nearest-neighbors"},
                 {"k-nearest-statistics", "k-nearest-neighbours-statistics"},
                 {"k-nearest-statistics", "k-nearest-neighbors-statistics"},
                 {"fail", "none"},
             }) {
            EXPECT(*make(test.name) == *make(test.alias));
        }
    }


    SECTION("unknown") {
        EXPECT_THROWS_AS(make("unknown"), exception::SeriousBug);
        EXPECT_THROWS_AS(make("{type: unknown}"), exception::SeriousBug);
    }


    SECTION("invalid options") {
        EXPECT_THROWS(make("{type: linear, prune-epsilon: -1}"));
        EXPECT_THROWS(make("{type: linear, lsm-weight-adjustment: -1}"));
        EXPECT_THROWS(make("{type: linear, pole-displacement-in-degree: -1}"));
        EXPECT_THROWS(make("{type: linear, finite-element-projection-fail: unknown}"));
        EXPECT_THROWS(make("{type: k-nearest, distance-weighting: unknown}"));
        EXPECT_THROWS(make("{type: k-nearest, nearest-method: unknown}"));
        EXPECT_THROWS(make("{type: linear, non-linear: unknown}"));
    }
}


CASE("Method spec, options common to matrix-based methods") {
    SECTION("non-default options are listed") {
        struct test_t {
            std::string options;
            std::string expected;
        };

        for (const auto& test : std::vector<test_t>{
                 {"prune-epsilon: 0.1", R"("prune-epsilon":0.1)"},
                 {"pole-displacement-in-degree: 0.5", R"("pole-displacement-in-degree":0.5)"},
                 {"lsm-weight-adjustment: 0.5", R"("lsm-weight-adjustment":0.5)"},
                 {"lsm: true", R"("lsm":true)"},
                 {"non-linear: [missing-if-all-missing, heaviest]",
                  R"("non-linear":["missing-if-all-missing","heaviest"])"},
             }) {
            auto m = make("{type: grid-box-average, " + test.options + "}");
            EXPECT(m->json_str().find(test.expected) != std::string::npos);
            EXPECT(*m != *make("grid-box-average"));
            EXPECT(*make(m->json_str()) == *m);
        }

        // lsm: false is the default
        EXPECT(*make("{type: grid-box-average, lsm: false}") == *make("grid-box-average"));
    }


    SECTION("complete specs list the defaults, the solver and the matrix reordering") {
        param::DefaultParametrisation::JSONComplete complete;

        auto linear = make("linear");
        EXPECT(linear->json_str() == linear->json_str(false));

        for (const auto& expected : std::vector<std::string>{
                 R"("prune-epsilon":1e-10)",
                 R"("pole-displacement-in-degree":0)",
                 R"("lsm-weight-adjustment":0.2)",
                 R"("solver":{"type":"multiply"})",
             }) {
            EXPECT(linear->json_str().find(expected) != std::string::npos);
        }

        auto statistics = make("grid-box-statistics");
        EXPECT(statistics->json_str().find(R"("solver":{"type":"statistics")") != std::string::npos);

        auto reorder = make(
            "{type: linear, finite-element-matrix-reorder-rows: identity, finite-element-matrix-reorder-cols: "
            "identity}");
        EXPECT(reorder->json_str().find(R"("reorder-rows":"identity","reorder-cols":"identity")") != std::string::npos);
    }
}


CASE("Method comparison, hash and print") {
    auto md5 = [](const method::Method& m) {
        eckit::MD5 h;
        m.hash(h);
        return h.digest();
    };

    auto str = [](const method::Method& m) {
        std::ostringstream s;
        s << m;
        return s.str();
    };


    SECTION("matrix-based methods") {
        struct test_t {
            std::string spec;
            std::string other;  // a different option
            std::string print;  // the start of the print
        };

        for (const auto& test : std::vector<test_t>{
                 {"linear", "{type: linear, prune-epsilon: 0.1}", "FiniteElement[type=linear,"},
                 {"bilinear", "{type: bilinear, finite-element-validate-mesh: true}", "FiniteElement[type=bilinear,"},
                 {"nn", "{type: nn, nearest-method: nclosest, nclosest: 1}", "KNearestNeighbours["},
                 {"k-nearest", "{type: k-nearest, distance-weighting: 'no'}", "KNearestNeighbours["},
                 {"grid-box-average", "{type: grid-box-average, pole-displacement-in-degree: 1}",
                  "GridBoxMethod[type=grid-box-average,"},
                 {"healpix-resample", "{type: healpix-resample, lsm-weight-adjustment: 0.5}", "HEALPixResample["},
                 {"restructure", "{type: restructure, nclosest: 2}", "Restructure["},
             }) {
            auto a = make(test.spec);
            auto b = make(test.spec);
            auto c = make(test.other);

            EXPECT(a->sameAs(*b));
            EXPECT(!a->sameAs(*c));
            EXPECT(md5(*a) == md5(*b));
            EXPECT(md5(*a) != md5(*c));
            EXPECT(str(*a).rfind(test.print, 0) == 0);
            EXPECT_EQUAL(a->version(), 0);
        }

        // different non-linear treatments
        EXPECT(!make("linear")->sameAs(*make("{type: linear, non-linear: missing-if-all-missing}")));

        // different options common to matrix-based methods, also for the k-nearest neighbours methods
        for (const auto& spec : std::vector<std::string>{"nn", "k-nearest"}) {
            auto a = make(spec);
            EXPECT(a->sameAs(*make(spec)));
            EXPECT(!a->sameAs(*make("{type: " + spec + ", prune-epsilon: 0.1}")));
            EXPECT(!a->sameAs(*make("{type: " + spec + ", lsm-weight-adjustment: 0.5}")));
            EXPECT(!a->sameAs(*make("{type: " + spec + ", non-linear: missing-if-all-missing}")));
        }

        // different matrix reordering
        auto reordered = make("{type: linear, finite-element-matrix-reorder-rows: healpix-ring-to-nested}");
        EXPECT(!make("linear")->sameAs(*reordered));
        EXPECT(md5(*make("linear")) != md5(*reordered));

        // different types
        EXPECT(!make("linear")->sameAs(*make("bilinear")));
        EXPECT(!make("nn")->sameAs(*make("k-nearest")));
        EXPECT(!make("grid-box-average")->sameAs(*make("linear")));
        EXPECT(!make("healpix-resample")->sameAs(*make("restructure")));
    }


    SECTION("statistics-based methods are data-dependent") {
        for (const auto& spec : std::vector<std::string>{"grid-box-statistics", "voronoi-statistics",
                                                         "healpix-statistics", "k-nearest-statistics"}) {
            auto a = make(spec);
            EXPECT(!a->sameAs(*a));
            EXPECT(*a == *make(spec));
            EXPECT(str(*a).find("Statistics[stats=") != std::string::npos);
        }

        EXPECT(md5(*make("grid-box-statistics")) != md5(*make("voronoi-statistics")));
    }


    SECTION("matrix-free methods") {
        auto a = make("structured-bilinear");
        auto b = make("{type: structured-bilinear, interpolation-matrix-free: false}");

        EXPECT(a->sameAs(*make("structured-bilinear")));
        EXPECT(!a->sameAs(*b));
        EXPECT(!a->sameAs(*make("structured-bicubic")));
        EXPECT(md5(*a) != md5(*b));
        EXPECT(str(*a).rfind("ProxyMethod[options=", 0) == 0);
        EXPECT_EQUAL(a->version(), 0);
    }
}


CASE("Method cropping") {
    const util::BoundingBox bbox{10., 0., -10., 20.};
    const util::BoundingBox other{20., 0., -10., 20.};


    SECTION("matrix-based and matrix-free methods") {
        for (const auto& spec : std::vector<std::string>{"linear", "grid-box-average", "nn", "structured-bilinear"}) {
            auto a = make(spec);
            auto b = make(spec);

            EXPECT(a->canCrop());
            EXPECT(!a->hasCropping());

            a->setCropping(bbox);
            EXPECT(a->hasCropping());
            EXPECT(a->getCropping() == bbox);
            EXPECT(!a->sameAs(*b));
            EXPECT(!b->sameAs(*a));

            b->setCropping(bbox);
            EXPECT(a->sameAs(*b));

            b->setCropping(other);
            EXPECT(!a->sameAs(*b));
        }
    }


    SECTION("matrix-based methods print the cropping") {
        auto a = make("linear");

        std::ostringstream before;
        before << *a;
        EXPECT(before.str().find("cropping=none") != std::string::npos);

        a->setCropping(bbox);

        std::ostringstream after;
        after << *a;
        EXPECT(after.str().find("cropping=none") == std::string::npos);
    }
}


CASE("Method fail (none)") {
    auto m = make("fail");

    EXPECT_EQUAL(m->json_str(), R"({"type":"fail"})");
    EXPECT(m->sameAs(*make("none")));
    EXPECT(!m->sameAs(*make("linear")));

    std::ostringstream str;
    str << *m;
    EXPECT_EQUAL(str.str(), "FailMethod[]");

    eckit::MD5 h;
    m->hash(h);

    EXPECT(!m->canCrop());
    EXPECT(!m->hasCropping());
    EXPECT_THROWS(m->getCropping());
    EXPECT_THROWS(m->setCropping(util::BoundingBox()));
    EXPECT_THROWS(m->version());

    auto in  = representation("{grid: [10, 10]}");
    auto out = representation("{grid: [20, 20]}");
    EXPECT_THROWS_AS(interpolate(*m, field(in, MIRValuesVector(in->numberOfPoints(), 1.)), *out), exception::UserError);
}


}  // namespace mir::interpolation::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
