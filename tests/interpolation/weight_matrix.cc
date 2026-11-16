// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <algorithm>
#include <cmath>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "eckit/filesystem/TmpDir.h"
#include "eckit/parser/YAMLParser.h"
#include "eckit/testing/Test.h"

#include "mir/api/MIRJob.h"
#include "mir/caching/WeightCache.h"
#include "mir/config/LibMir.h"
#include "mir/input/GridSpecInput.h"
#include "mir/output/EmptyOutput.h"

#include "interpolation.h"


namespace mir::interpolation::tests {


using method::WeightMatrix;


CASE("WeightMatrix::validate") {
    SECTION("out-of-bounds") {
        const auto* when{"out-of-bounds"};
        const std::string what{
            "Invalid weight matrix (out-of-bounds): 1 row error, "
            "row 2: weights out-of-bounds, contents: (2,2,-0.1)"};

        WeightMatrix W(3, 3);
        W.setFromTriplets({{0, 0, 1.}, {1, 1, 0.}, {2, 2, -0.1}});

        try {
            W.validate(when, {false, true, false});
            ASSERT(false);
        }
        catch (exception::InvalidWeightMatrix& e) {
            EXPECT(e.what() == what);
        }
    }


    SECTION("weights sum") {
        const auto* when{"weights sum"};
        const std::string what{
            "Invalid weight matrix (weights sum): 2 row errors, "
            "row 1: weights sum not 0 or 1 (sum=0.5, 1-sum=0.5), contents: (1,1,0.5), "
            "row 2: weights sum not 0 or 1 (sum=0.1, 1-sum=0.9), contents: (2,2,0.1)"};

        WeightMatrix W(3, 3);
        W.setFromTriplets({{0, 0, 1.}, {1, 1, 0.5}, {2, 2, 0.1}});

        try {
            W.validate(when, {false, false, true});
            ASSERT(false);
        }
        catch (exception::InvalidWeightMatrix& e) {
            EXPECT(e.what() == what);
        }
    }


    SECTION("duplicate indices") {
        const auto* when{"duplicate indices"};
        const std::string what{
            "Invalid weight matrix (duplicate indices): 1 row error, "
            "row 1: duplicate indices, contents: (1,1,0.5), (1,1,0.5)"};

        WeightMatrix W(3, 3);
        W.setFromTriplets({{0, 0, 1.}, {1, 1, 0.5}, {1, 1, 0.5}});

        try {
            W.validate(when, {true, false, false});
            ASSERT(false);
        }
        catch (exception::InvalidWeightMatrix& e) {
            EXPECT(e.what() == what);
        }
    }


    SECTION("mixed") {
        const auto* when{"mixed"};
        const std::string what{
            "Invalid weight matrix (mixed): 2 row errors, "
            "row 0: duplicate indices, contents: (0,0,0.5), (0,0,0.5), "
            "row 1: weights sum not 0 or 1 (sum=0.5, 1-sum=0.5), contents: (1,1,0.5)"};

        WeightMatrix W(3, 3);
        W.setFromTriplets({{0, 0, 0.5}, {0, 0, 0.5}, {1, 1, 0.5}});

        try {
            W.validate(when, {true, false, true});
            ASSERT(false);
        }
        catch (exception::InvalidWeightMatrix& e) {
            EXPECT(e.what() == what);
        }
    }
}


CASE("interpolation-matrix, also from the in-memory cache") {
    const eckit::TmpDir dir;

    input::GridSpecInput input("{grid: H4n}");
    output::EmptyOutput output;

    for (const auto& name : std::vector<std::string>{"a.mat", "b.mat"}) {
        const auto path = dir / name;

        api::MIRJob job;
        job.set("grid", "O4");
        job.set("interpolation", "nn");
        job.set("interpolation-matrix", path.asString());
        job.execute(input, output);

        EXPECT(path.exists());
    }


    SECTION("method matrix, interpolating with a saved matrix") {
        auto in  = representation("{grid: H4n}");
        auto out = representation("{grid: O4}");

        auto values = field(in, [](double lat, double lon) { return lat + 0.1 * lon; });
        auto a      = interpolate(*make("nn"), values, *out);

        for (const auto& name : std::vector<std::string>{"a.mat", "b.mat"}) {
            const auto spec = "{type: matrix, interpolation-matrix: '" + (dir / name).asString() + "'}";
            auto m          = make(spec);

            EXPECT(m->json_str().find(R"("interpolation-matrix":")") != std::string::npos);
            EXPECT(m->sameAs(*make(spec)));
            EXPECT(!m->sameAs(*make("nn")));

            std::ostringstream str;
            str << *m;
            EXPECT(str.str().rfind("Matrix[", 0) == 0);

            auto b = interpolate(*m, values, *out);
            EXPECT(a.values(0) == b.values(0));
        }

        EXPECT(!make("{type: matrix, interpolation-matrix: '" + (dir / "a.mat").asString() + "'}")
                    ->sameAs(*make("{type: matrix, interpolation-matrix: '" + (dir / "b.mat").asString() + "'}")));
    }


    SECTION("method matrix, invalid options") {
        EXPECT_THROWS_AS(make("matrix"), exception::UserError);
        EXPECT_THROWS_AS(make("{type: matrix, interpolation-matrix: '" + (dir / "none.mat").asString() + "'}"),
                         exception::UserError);
    }
}


CASE("WeightMatrix") {
    WeightMatrix W(2, 3);
    W.setFromTriplets({{0, 0, 0.6}, {0, 1, 0.35}, {0, 2, 0.05}, {1, 2, 1.}});

    EXPECT_EQUAL(W.rows(), 2);
    EXPECT_EQUAL(W.cols(), 3);
    EXPECT_EQUAL(W.nonZeros(), 4);
    EXPECT_NO_THROW(W.validate("valid", {true, true, true}));


    SECTION("cleanup: small weights are removed, redistributed to the others") {
        W.cleanup(0.1);

        EXPECT_EQUAL(W.nonZeros(), 3);
        EXPECT(same(weight(W, 0, 0), 0.625));
        EXPECT(same(weight(W, 0, 1), 0.375));
        EXPECT(same(weight(W, 1, 2), 1.));
        EXPECT_NO_THROW(W.validate("cleanup", {true, true, true}));
    }


    SECTION("save, load") {
        const eckit::TmpDir dir;
        const auto path = dir / "W.mat";

        W.save(path);
        WeightMatrix L(path);

        EXPECT_EQUAL(L.rows(), W.rows());
        EXPECT_EQUAL(L.cols(), W.cols());
        EXPECT_EQUAL(L.nonZeros(), W.nonZeros());
        for (size_t r = 0; r < W.rows(); ++r) {
            for (size_t c = 0; c < W.cols(); ++c) {
                EXPECT(weight(L, r, c) == weight(W, r, c));
            }
        }
    }


    SECTION("print") {
        std::ostringstream str;
        str << W;
        EXPECT(str.str().rfind("WeightMatrix[", 0) == 0);
    }
}


CASE("matrix-based methods, matrix caching") {
    auto in  = representation("{grid: [10, 10], area: [40, 0, -40, 90]}");
    auto out = representation("{grid: [7, 7], area: [35, 7, -35, 84]}");

    auto values = field(in, [](double lat, double lon) { return lat + 0.1 * lon; });

    data::MIRField dummy(in);
    util::MIRStatistics statistics;
    context::Context ctx(dummy, statistics);


    SECTION("in-memory cache") {
        auto m = make("{type: linear, caching: false}");

        const auto* w = dynamic_cast<const method::MethodWeighted*>(m.get());
        ASSERT(w != nullptr);

        const auto& a = w->getMatrix(ctx, *in, *out);
        const auto& b = w->getMatrix(ctx, *in, *out);
        EXPECT(&a == &b);

        // the same method, built separately
        auto other    = make("linear");
        const auto& c = dynamic_cast<const method::MethodWeighted&>(*other).getMatrix(ctx, *in, *out);
        EXPECT(&a == &c);
    }


    SECTION("disk cache (the same matrix)") {
        // (the matrix is created if not found)
        std::vector<eckit::PathName> files;
        std::vector<eckit::PathName> dirs;
        eckit::PathName cache(LibMir::cacheDir() + "/mir/weights/" + std::to_string(caching::WeightCache::version()) +
                              "/k-nearest/" + in->uniqueName());
        if (cache.exists()) {
            cache.children(files, dirs);
            for (const auto& file : files) {
                file.unlink();
            }
        }

        // (pole-displacement-in-degree differs, so these are not in memory)
        auto a =
            interpolate(*make("{type: k-nearest, pole-displacement-in-degree: 1e-9, caching: false}"), values, *out);
        auto b =
            interpolate(*make("{type: k-nearest, pole-displacement-in-degree: 2e-9, caching: true}"), values, *out);

        for (size_t i = 0; i < out->numberOfPoints(); ++i) {
            EXPECT(same(a.values(0)[i], b.values(0)[i]));
        }
    }


    SECTION("information about the matrix") {
        const eckit::TmpDir dir;
        const auto path = (dir / "info.json").asString();

        auto info = [&path](const std::string& key) {
            auto value = eckit::YAMLParser::decodeFile(path);
            std::ostringstream str;
            str << value[key];
            return str.str();
        };

        auto lam = representation("{grid: [5, 5], area: [10, 0, 0, 10]}");

        struct test_t {
            std::string spec;
            std::string interpolation;
        };

        for (const auto& test : std::vector<test_t>{
                 {"{type: linear}", "linear"},
                 {"{type: nn}", "nearest-neighbour"},
                 {"{type: grid-box-average}", "grid-box-average"},
                 {"{type: linear, finite-element-validate-mesh: true}", ""},
                 {"{type: nn, nclosest: 2}", ""},
                 {"{type: grid-box-average, prune-epsilon: 0.01}", ""},
             }) {
            auto m = make(test.spec.substr(0, test.spec.size() - 1) + ", dump-weights-info: '" + path + "'}");

            // (a different output, so the matrix is computed)
            auto output = representation("{grid: [2, 2], area: [10, 0, 0, 10]}");
            interpolate(*m, field(lam, MIRValuesVector(lam->numberOfPoints(), 0.)), *output);

            if (test.interpolation.empty()) {
                EXPECT(info("interpolation").find("type => ") != std::string::npos);
            }
            else {
                EXPECT_EQUAL(info("interpolation"), test.interpolation);
            }

            auto value = eckit::YAMLParser::decodeFile(path);
            EXPECT_EQUAL(static_cast<size_t>(value["matrix"]["shape"][0]), output->numberOfPoints());
            EXPECT_EQUAL(static_cast<size_t>(value["matrix"]["shape"][1]), lam->numberOfPoints());
            EXPECT(static_cast<size_t>(value["matrix"]["nnz"]) > 0);
            EXPECT_EQUAL(static_cast<int>(value["matrix"]["version"]), caching::WeightCache::version());
        }
    }


    SECTION("interpolating angles (vector space)") {
        auto two = representation("{grid: [10, 10], area: [0, 0, 0, 10]}");
        auto mid = representation("{grid: [10, 10], area: [0, 5, 0, 5]}");

        auto m = make(
            "{type: k-nearest, nearest-method: nclosest, nclosest: 2, distance-weighting: 'no', vector-space: "
            "1d-angle-degree-asymmetric}");

        auto result = interpolate(*m, field(two, MIRValuesVector{350., 10.}), *mid);
        for (const auto& v : result.values(0)) {
            EXPECT(std::min(std::abs(v), std::abs(v - 360.)) < 1e-9);
        }
    }
}


}  // namespace mir::interpolation::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
