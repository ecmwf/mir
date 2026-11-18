// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "eckit/log/JSON.h"
#include "eckit/testing/Test.h"
#include "eckit/utils/MD5.h"

#include "mir/method/nonlinear/NonLinear.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/util/Log.h"

#include "interpolation.h"


namespace mir::interpolation::tests {


using method::DenseMatrix;
using method::WeightMatrix;
using method::nonlinear::NonLinear;
using method::nonlinear::NonLinearFactory;

using N = std::unique_ptr<const NonLinear>;


constexpr double MISSING = 9999.;


/*
 * a 3x4 matrix, as rows: [0.5 0.3 0.2 0], [0 0.6 0 0.4], [0 0 1 0], and the input values (column 1 missing):
 * - row 0, the missing value is not the heaviest
 * - row 1, the missing value is the heaviest
 * - row 2, no missing values
 */
WeightMatrix matrix_3x4() {
    WeightMatrix W(3, 4);
    W.setFromTriplets({{0, 0, 0.5}, {0, 1, 0.3}, {0, 2, 0.2}, {1, 1, 0.6}, {1, 3, 0.4}, {2, 2, 1.}});
    return W;
}


const MIRValuesVector VALUES{1., MISSING, 3., 4.};


struct Expected {
    std::string name;
    bool modified;
    std::vector<std::vector<double>> rows;  // dense rows, after treatment
};


bool treat(const NonLinear& n, WeightMatrix& W, const MIRValuesVector& values, double missingValue = MISSING) {
    DenseMatrix A;
    DenseMatrix B;
    return n.treatment(A, W, B, values, missingValue);
}


bool rows_equal(const WeightMatrix& W, const std::vector<std::vector<double>>& rows) {
    for (size_t r = 0; r < rows.size(); ++r) {
        for (size_t c = 0; c < rows[r].size(); ++c) {
            if (!same(weight(W, r, c), rows[r][c])) {
                Log::info() << "row " << r << " col " << c << ": " << weight(W, r, c) << " != " << rows[r][c]
                            << std::endl;
                return false;
            }
        }
    }
    return true;
}


CASE("non-linear treatments of the weights") {
    param::SimpleParametrisation param;

    const double a = 0.5 / 0.7;
    const double b = 0.2 / 0.7;


    SECTION("by name, on a matrix with missing values") {
        for (const auto& test : std::vector<Expected>{
                 {"missing-if-heaviest-missing", true, {{a, 0., b, 0.}, {0., 1., 0., 0.}, {0., 0., 1., 0.}}},
                 {"missing-if-all-missing", true, {{a, 0., b, 0.}, {0., 0., 0., 1.}, {0., 0., 1., 0.}}},
                 {"missing-if-any-missing", true, {{0., 1., 0., 0.}, {0., 1., 0., 0.}, {0., 0., 1., 0.}}},
                 {"heaviest", true, {{1., 0., 0., 0.}, {0., 1., 0., 0.}, {0., 0., 1., 0.}}},
                 {"no", false, {{0.5, 0.3, 0.2, 0.}, {0., 0.6, 0., 0.4}, {0., 0., 1., 0.}}},
             }) {
            N n(NonLinearFactory::build(test.name, param));
            EXPECT_EQUAL(n->name(), test.name);

            auto W = matrix_3x4();
            EXPECT_EQUAL(treat(*n, W, VALUES), test.modified);
            EXPECT(rows_equal(W, test.rows));

            // the structure (non-zeros) is kept
            EXPECT_EQUAL(W.nonZeros(), 6);
        }
    }


    SECTION("by name, on a matrix without missing values") {
        for (const auto& name : std::vector<std::string>{"missing-if-heaviest-missing", "missing-if-all-missing",
                                                         "missing-if-any-missing", "simulated-missing-value", "no"}) {
            N n(NonLinearFactory::build(name, param));

            auto W = matrix_3x4();
            EXPECT(!treat(*n, W, {1., 2., 3., 4.}));
            EXPECT(rows_equal(W, {{0.5, 0.3, 0.2, 0.}, {0., 0.6, 0., 0.4}, {0., 0., 1., 0.}}));
        }
    }


    SECTION("all values of a row missing, or the remaining weights summing zero") {
        WeightMatrix W(2, 3);
        W.setFromTriplets({{0, 0, 0.5}, {0, 1, 0.5}, {1, 1, 1.}, {1, 2, 0.}});

        // row 0: all missing; row 1: the only non-missing value has weight zero
        const MIRValuesVector values{MISSING, MISSING, 1.};

        for (const auto& name : std::vector<std::string>{"missing-if-heaviest-missing", "missing-if-all-missing",
                                                         "missing-if-any-missing"}) {
            N n(NonLinearFactory::build(name, param));

            auto M = W;
            EXPECT(treat(*n, M, values));

            // the (last) missing value of each row takes all the weight, so the result is missing
            EXPECT(rows_equal(M, {{0., 1., 0.}, {0., 1., 0.}}));
        }
    }


    SECTION("modifies the matrix") {
        struct test_t {
            std::string name;
            bool withMissing;
            bool withoutMissing;
        };

        for (const auto& test : std::vector<test_t>{
                 {"missing-if-heaviest-missing", true, false},
                 {"missing-if-all-missing", true, false},
                 {"missing-if-any-missing", true, false},
                 {"heaviest", true, true},
                 {"simulated-missing-value", true, true},
                 {"no", false, false},
             }) {
            N n(NonLinearFactory::build(test.name, param));
            EXPECT_EQUAL(n->modifiesMatrix(true), test.withMissing);
            EXPECT_EQUAL(n->modifiesMatrix(false), test.withoutMissing);
        }
    }


    SECTION("comparison, hash, print and json") {
        const std::vector<std::string> names{"missing-if-heaviest-missing", "missing-if-all-missing",
                                             "missing-if-any-missing",      "heaviest",
                                             "simulated-missing-value",     "no"};

        for (const auto& name : names) {
            N n(NonLinearFactory::build(name, param));

            for (const auto& other : names) {
                N o(NonLinearFactory::build(other, param));
                EXPECT(n->sameAs(*o) == (name == other));

                eckit::MD5 h1;
                eckit::MD5 h2;
                n->hash(h1);
                o->hash(h2);
                EXPECT((h1.digest() == h2.digest()) == (name == other));
            }

            std::ostringstream str;
            str << *n;
            EXPECT(!str.str().empty());

            std::ostringstream js;
            eckit::JSON j(js);
            j.startObject();
            j << *n;
            j.endObject();
            EXPECT((js.str() == "{}") == (name != "simulated-missing-value"));
        }
    }


    SECTION("unknown") {
        EXPECT_THROWS_AS(N(NonLinearFactory::build("unknown", param)), exception::SeriousBug);
    }
}


CASE("simulated missing value") {
    param::SimpleParametrisation param;
    param.set("simulated-missing-value", 100.);
    param.set("simulated-missing-value-epsilon", 1.);

    N n(NonLinearFactory::build("simulated-missing-value", param));


    SECTION("treatment, by value within epsilon (ignoring the field missing value)") {
        const double a = 0.5 / 0.7;
        const double b = 0.2 / 0.7;

        auto W = matrix_3x4();
        EXPECT(treat(*n, W, {1., 100.5, 3., 4.}, MISSING));
        EXPECT(rows_equal(W, {{a, 0., b, 0.}, {0., 1., 0., 0.}, {0., 0., 1., 0.}}));

        // all missing
        WeightMatrix M(1, 2);
        M.setFromTriplets({{0, 0, 0.5}, {0, 1, 0.5}});
        EXPECT(treat(*n, M, {99.5, 100.}, MISSING));
        EXPECT(rows_equal(M, {{0., 1.}}));
    }


    SECTION("comparison, print and json") {
        param::SimpleParametrisation other;
        N d(NonLinearFactory::build("simulated-missing-value", other));

        EXPECT(n->sameAs(*n));
        EXPECT(!n->sameAs(*d));

        std::ostringstream str;
        str << *n;
        EXPECT_EQUAL(str.str(), "SimulatedMissingValue[missingValue=100,epsilon=1]");

        std::ostringstream js;
        eckit::JSON j(js);
        j.startObject();
        j << *n;
        j.endObject();
        EXPECT_EQUAL(js.str(), R"({"simulated-missing-value":100,"simulated-missing-value-epsilon":1})");
    }
}


CASE("non-linear treatments, interpolating") {
    // 3x3 points to the centres of the 2x2 cells, averaging the 4 closest points (equidistant)
    auto in  = representation("{grid: [10, 10], area: [10, 0, -10, 20]}");
    auto out = representation("{grid: [10, 10], area: [5, 5, -5, 15]}");
    ASSERT(in->numberOfPoints() == 9);
    ASSERT(out->numberOfPoints() == 4);

    // values (row-major from north-west), the centre missing
    const MIRValuesVector values{1., 2., 3., 4., MISSING, 6., 7., 8., 9.};

    const std::string spec =
        "{type: k-nearest, nearest-method: nclosest, nclosest: 4, distance-weighting: 'no', caching: false, "
        "non-linear: ";


    SECTION("missing-if-all-missing: average of the non-missing values") {
        auto m      = make(spec + "missing-if-all-missing}");
        auto result = interpolate(*m, field(in, values), *out);

        EXPECT(count_missing(result) == 0);
        EXPECT(same(result.values(0)[0], (1. + 2. + 4.) / 3.));
        EXPECT(same(result.values(0)[3], (6. + 8. + 9.) / 3.));
    }


    SECTION("missing-if-any-missing: all missing") {
        auto m      = make(spec + "missing-if-any-missing}");
        auto result = interpolate(*m, field(in, values), *out);

        EXPECT(result.hasMissing());
        EXPECT(count_missing(result) == 4);
    }


    SECTION("no (non-linear treatment): the missing value is interpolated as a value") {
        auto m      = make(spec + "'no'}");
        auto result = interpolate(*m, field(in, values), *out);

        EXPECT(same(result.values(0)[0], (1. + 2. + 4. + MISSING) / 4.));
    }


    SECTION("several treatments, in sequence") {
        auto m = make(spec + "[missing-if-all-missing, heaviest]}");
        EXPECT(m->json_str().find(R"("non-linear":["missing-if-all-missing","heaviest"])") != std::string::npos);

        // heaviest picks one of the (equal) remaining weights, so the result is an input value
        auto result = interpolate(*m, field(in, values), *out);

        EXPECT(count_missing(result) == 0);
        for (const auto& v : result.values(0)) {
            EXPECT(v == 1. || v == 2. || v == 4. || v == 6. || v == 8. || v == 9. || v == 3. || v == 7.);
        }
    }


    SECTION("simulated-missing-value, on a field without missing values") {
        auto m = make(spec + "simulated-missing-value, simulated-missing-value: 9999}");

        auto f = field(in, values, -1.);  // a missing value not present
        EXPECT(!f.hasMissing());

        auto result = interpolate(*m, f, *out);
        EXPECT(same(result.values(0)[0], (1. + 2. + 4.) / 3.));
    }
}


}  // namespace mir::interpolation::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
