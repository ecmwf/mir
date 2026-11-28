// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <algorithm>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "eckit/testing/Test.h"

#include "mir/util/Reorder.h"

#include "interpolation.h"


namespace mir::interpolation::tests {


const std::vector<std::string> FINITE_ELEMENT{"linear", "bilinear"};


auto smooth = [](double lat, double lon) { return 10. + lat + 0.1 * lon; };


bool bounded(const data::MIRField& input, const data::MIRField& output, double eps = 1e-9) {
    const auto& in = input.values(0);
    auto min       = *std::min_element(in.begin(), in.end());
    auto max       = *std::max_element(in.begin(), in.end());

    const auto& out = output.values(0);
    return std::all_of(out.begin(), out.end(), [=](double v) { return min - eps <= v && v <= max + eps; });
}


CASE("finite element methods") {
    auto in  = representation("{grid: [5, 5]}");
    auto out = representation("{grid: O16}");


    SECTION("a constant is reproduced") {
        for (const auto& spec : FINITE_ELEMENT) {
            auto result = interpolate(*make(spec), field(in, MIRValuesVector(in->numberOfPoints(), 3.)), *out);

            EXPECT(!result.hasMissing());
            for (const auto& v : result.values(0)) {
                EXPECT(same(v, 3., 1e-9));
            }
        }
    }


    SECTION("results are bounded by the input values") {
        auto input = field(in, smooth);

        for (const auto& spec : FINITE_ELEMENT) {
            auto result = interpolate(*make(spec), input, *out);
            EXPECT(bounded(input, result));
        }
    }


    SECTION("weights are valid (no duplicates, bounded, summing 1)") {
        for (const auto& spec : FINITE_ELEMENT) {
            auto W = matrix(*make(spec), *in, *out);
            EXPECT_EQUAL(W.rows(), out->numberOfPoints());
            EXPECT_EQUAL(W.cols(), in->numberOfPoints());
            EXPECT_NO_THROW(W.validate(spec.c_str(), {true, true, true}));

            // linear: triangles (up to 3 weights per row), bilinear: also quadrilaterals (up to 4)
            size_t maxPerRow = 0;
            for (size_t r = 0; r < W.rows(); ++r) {
                size_t n = 0;
                for (auto it = W.begin(r); it != W.end(r); ++it) {
                    ++n;
                }
                maxPerRow = std::max(maxPerRow, n);
            }
            EXPECT(maxPerRow == (spec == "linear" ? 3 : 4));
        }
    }


    SECTION("output points coinciding with input points") {
        auto subset = representation("{grid: [10, 10], area: [40, 0, -40, 90]}");
        auto input  = field(in, smooth);

        for (const auto& spec : FINITE_ELEMENT) {
            auto result = interpolate(*make(spec), input, *subset);

            size_t i           = 0;
            const auto& points = subset->canonicalGridUnrotated();
            for (auto it = points.cbegin(); it; ++it, ++i) {
                const auto p = std::get<PointLonLat>(*it);
                EXPECT(same(result.values(0)[i], smooth(p.lat(), p.lon()), 1e-6));
            }
        }
    }


    SECTION("the same input and output: identity") {
        auto W = matrix(*make("linear"), *in, *in);

        EXPECT_EQUAL(W.nonZeros(), in->numberOfPoints());
        for (size_t r = 0; r < W.rows(); ++r) {
            EXPECT(weight(W, r, r) == 1.);
        }
    }


    SECTION("output points outside the input domain are missing") {
        auto lam    = representation("{grid: [5, 5], area: [20, 0, -20, 40]}");
        auto result = interpolate(*make("linear"), field(lam, smooth), *out);

        EXPECT(result.hasMissing());
        EXPECT(count_missing(result) < out->numberOfPoints());
    }


    SECTION("input without a known longest element diagonal (computed from the mesh)") {
        auto healpix = representation("{grid: H8}");
        auto input   = field(healpix, smooth);

        auto result = interpolate(*make("linear"), input, *out);
        EXPECT(!result.hasMissing());
        EXPECT(bounded(input, result));
    }


    SECTION("mesh validation") {
        auto input = field(in, smooth);

        for (const auto& spec : FINITE_ELEMENT) {
            auto result = interpolate(*make("{type: " + spec + ", finite-element-validate-mesh: true}"), input, *out);
            EXPECT(bounded(input, result));
        }
    }


    SECTION("projection fail options") {
        auto input = field(in, smooth);

        for (const auto& option : std::vector<std::string>{"increase-epsilon", "missing-value", "failure"}) {
            auto m = make("{type: linear, finite-element-projection-fail: " + option + "}");

            auto result = interpolate(*m, input, *out);
            EXPECT(!result.hasMissing());
            EXPECT(bounded(input, result));

            // the spec, rebuilt
            EXPECT(m->json_str(false).find(R"("finite-element-projection-fail":")" + option + "\"") !=
                   std::string::npos);
            EXPECT(*make(m->json_str()) == *m);
        }
    }


    SECTION("matrix reordering") {
        auto healpix = representation("{grid: H4}");
        auto input   = field(in, smooth);

        // (the matrices are cached differently)
        auto a = interpolate(*make("linear"), input, *healpix);
        auto b = interpolate(*make("{type: linear, finite-element-matrix-reorder-rows: healpix-ring-to-nested, "
                                   "finite-element-matrix-reorder-cols: identity}"),
                             input, *healpix);

        std::unique_ptr<util::Reorder> reorder(
            util::Reorder::build("healpix-ring-to-nested", healpix->numberOfPoints()));
        auto rows = reorder->reorder();

        for (size_t r = 0; r < rows.size(); ++r) {
            EXPECT(same(b.values(0)[rows[r]], a.values(0)[r]));
        }

        std::ostringstream str;
        str << *make("{type: linear, finite-element-matrix-reorder-rows: identity}");
        EXPECT(str.str().find("reorderRows=identity") != std::string::npos);
    }
}


}  // namespace mir::interpolation::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
