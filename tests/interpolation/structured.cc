// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <cmath>
#include <memory>
#include <string>
#include <vector>

#include "eckit/testing/Test.h"

#include "interpolation.h"


namespace mir::interpolation::tests {


const std::vector<std::string> STRUCTURED{"structured-bilinear", "structured-bicubic", "structured-biquasicubic"};


CASE("structured methods (Atlas)") {
    auto in  = representation("{grid: [5, 5]}");
    auto out = representation("{grid: O16}");


    SECTION("a constant is reproduced") {
        for (const auto& spec : STRUCTURED) {
            for (const auto& matrixFree : std::vector<std::string>{"true", "false"}) {
                auto m      = make("{type: " + spec + ", interpolation-matrix-free: " + matrixFree + "}");
                auto result = interpolate(*m, field(in, MIRValuesVector(in->numberOfPoints(), 3.)), *out);

                EXPECT_EQUAL(result.values(0).size(), out->numberOfPoints());
                for (const auto& v : result.values(0)) {
                    EXPECT(same(v, 3., 1e-9));
                }
            }
        }
    }


    SECTION("a function linear in latitude is reproduced") {
        auto f = [](double lat, double) { return 10. + lat; };

        for (const auto& spec : STRUCTURED) {
            auto result = interpolate(*make(spec), field(in, f), *out);

            // (bicubic stencils near the poles span across the pole, where the function is not linear)
            size_t i           = 0;
            const auto& points = out->canonicalGridUnrotated();
            for (auto it = points.cbegin(); it; ++it, ++i) {
                const auto lat = PointLatLon{std::get<PointLonLat>(*it)}.lat().value();
                if (spec == "structured-bilinear" || std::abs(lat) < 85.) {
                    EXPECT(same(result.values(0)[i], f(lat, 0.), 1e-9));
                }
            }
        }
    }


    SECTION("multiple fields") {
        data::MIRField two(in, false, 9999.);
        MIRValuesVector a(in->numberOfPoints(), 1.);
        MIRValuesVector b(in->numberOfPoints(), 2.);
        two.update(a, 0);
        two.update(b, 1);

        auto result = interpolate(*make("structured-bilinear"), two, *out);
        EXPECT_EQUAL(result.dimensions(), 2);
        EXPECT(same(result.values(0).front(), 1.));
        EXPECT(same(result.values(1).back(), 2.));
    }
}


CASE("grid-box methods, matrix-free (Atlas)") {
    auto in  = representation("{grid: [2, 2]}");
    auto out = representation("{grid: [6, 6]}");

    for (const auto& spec : std::vector<std::string>{"grid-box-average-matrix-free", "grid-box-maximum-matrix-free"}) {
        auto m = make(spec);
        EXPECT_EQUAL(m->json_str(false), R"({"type":")" + spec + R"(","interpolation-matrix-free":true})");

        auto result = interpolate(*m, field(in, MIRValuesVector(in->numberOfPoints(), 3.)), *out);
        for (const auto& v : result.values(0)) {
            EXPECT(same(v, 3., 1e-9));
        }
    }
}


}  // namespace mir::interpolation::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
