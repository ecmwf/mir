// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <algorithm>
#include <memory>
#include <numeric>
#include <string>
#include <vector>

#include "eckit/testing/Test.h"

#include "interpolation.h"


namespace mir::interpolation::tests {


auto smooth = [](double lat, double lon) { return 10. + lat + 0.1 * lon; };


double mean(const MIRValuesVector& values) {
    return std::accumulate(values.begin(), values.end(), 0.) / static_cast<double>(values.size());
}


bool contains(const MIRValuesVector& values, double value) {
    return std::find(values.begin(), values.end(), value) != values.end();
}


CASE("healpix-resample") {
    auto H2 = representation("{grid: H2}");
    auto H4 = representation("{grid: H4}");
    auto m  = make("healpix-resample");


    SECTION("downsampling: average of the children") {
        auto input  = field(H4, smooth);
        auto result = interpolate(*m, input, *H2);

        EXPECT_EQUAL(result.values(0).size(), H2->numberOfPoints());
        EXPECT(same(mean(result.values(0)), mean(input.values(0)), 1e-9));

        // in nested order, each value is the average of 4 consecutive input values
        auto H4n = representation("{grid: H4, order: nested}");
        auto H2n = representation("{grid: H2, order: nested}");

        MIRValuesVector values(H4n->numberOfPoints());
        std::iota(values.begin(), values.end(), 0.);

        auto nested = interpolate(*m, field(H4n, values), *H2n);
        for (size_t i = 0; i < H2n->numberOfPoints(); ++i) {
            EXPECT(same(nested.values(0)[i], 4. * static_cast<double>(i) + 1.5));
        }
    }


    SECTION("upsampling: copy of the parent") {
        auto input  = field(H2, smooth);
        auto result = interpolate(*m, input, *H4);

        EXPECT(same(mean(result.values(0)), mean(input.values(0)), 1e-9));
        for (const auto& v : result.values(0)) {
            EXPECT(contains(input.values(0), v));
        }

        // and back
        auto back = interpolate(*m, result, *H2);
        for (size_t i = 0; i < H2->numberOfPoints(); ++i) {
            EXPECT(same(back.values(0)[i], input.values(0)[i], 1e-9));
        }
    }


    SECTION("ring and nested orders") {
        auto ring   = field(H4, smooth);
        auto nested = representation("{grid: H4, order: nested}");

        // the same field, in the other order
        auto a = interpolate(*m, ring, *nested);
        auto b = interpolate(*m, a, *H2);
        auto c = interpolate(*m, ring, *H2);

        for (size_t i = 0; i < H2->numberOfPoints(); ++i) {
            EXPECT(same(b.values(0)[i], c.values(0)[i], 1e-9));
        }
    }


    SECTION("representations should be HEALPix") {
        auto ll = representation("{grid: [30, 30]}");
        EXPECT_THROWS_AS(interpolate(*m, field(ll, smooth), *H2), exception::SeriousBug);
        EXPECT_THROWS_AS(interpolate(*m, field(H2, smooth), *ll), exception::SeriousBug);
    }
}


CASE("healpix-statistics") {
    auto H2    = representation("{grid: H2}");
    auto H4    = representation("{grid: H4}");
    auto input = field(H4, smooth);

    auto average = interpolate(*make("healpix-resample"), input, *H2);
    auto maximum = interpolate(*make("healpix-statistics"), input, *H2);
    auto minimum = interpolate(*make("{type: healpix-statistics, interpolation-statistics: minimum}"), input, *H2);

    for (size_t i = 0; i < H2->numberOfPoints(); ++i) {
        EXPECT(minimum.values(0)[i] <= average.values(0)[i] + 1e-9);
        EXPECT(average.values(0)[i] <= maximum.values(0)[i] + 1e-9);
        EXPECT(contains(input.values(0), maximum.values(0)[i]));
        EXPECT(contains(input.values(0), minimum.values(0)[i]));
    }

    auto m = make("{type: healpix-statistics, interpolation-statistics: minimum}");
    EXPECT(m->json_str().find(R"("interpolation-statistics":"minimum")") != std::string::npos);
    EXPECT(*make(m->json_str()) == *m);
}


CASE("voronoi-statistics") {
    auto fine   = representation("{grid: [2, 2]}");
    auto coarse = representation("{grid: [10, 10]}");


    SECTION("to a coarser grid: statistics of the input points closest to each output point") {
        auto input   = field(fine, smooth);
        auto maximum = interpolate(*make("voronoi-statistics"), input, *coarse);
        auto minimum =
            interpolate(*make("{type: voronoi-statistics, interpolation-statistics: minimum}"), input, *coarse);

        EXPECT(!maximum.hasMissing());
        for (size_t i = 0; i < coarse->numberOfPoints(); ++i) {
            EXPECT(minimum.values(0)[i] <= maximum.values(0)[i]);
            EXPECT(contains(input.values(0), maximum.values(0)[i]));
        }
    }


    SECTION("to a finer grid: completed by the input points closest to the output points") {
        auto input  = field(coarse, smooth);
        auto result = interpolate(*make("voronoi-statistics"), input, *fine);

        EXPECT(!result.hasMissing());
        for (const auto& v : result.values(0)) {
            EXPECT(contains(input.values(0), v));
        }
    }


    SECTION("weights are valid (no duplicates, bounded)") {
        auto W = matrix(*make("voronoi-statistics"), *fine, *coarse);
        EXPECT_NO_THROW(W.validate("voronoi-statistics", {true, true, false}));

        // every input point contributes to (at least) one output point, several if equidistant
        EXPECT(W.nonZeros() >= fine->numberOfPoints());
    }


    SECTION("spec") {
        auto m = make("{type: voronoi-statistics, interpolation-statistics: minimum}");
        EXPECT(m->json_str().find(R"("interpolation-statistics":"minimum")") != std::string::npos);
        EXPECT(*make(m->json_str()) == *m);
    }
}


CASE("restructure") {
    auto in  = representation("{grid: [10, 10]}");
    auto out = representation("{grid: [5, 5]}");
    auto m   = make("restructure");

    auto input = field(in, smooth);


    SECTION("to a finer grid: output points not picked by any input point are missing") {
        auto result = interpolate(*m, input, *out);

        // 17x36 non-pole input points, and all 72 output points at each pole
        EXPECT(result.hasMissing());
        EXPECT_EQUAL(out->numberOfPoints() - count_missing(result), 17 * 36 + 2 * 72);

        size_t i = 0;
        for (std::unique_ptr<repres::Iterator> it(out->iterator()); it->next(); ++i) {
            const auto v = result.values(0)[i];
            if (v != result.missingValue()) {
                const auto& p = it->pointUnrotated();
                auto lat      = p.lat().value();

                // (at the poles, the value of the first input point)
                EXPECT(same(v, smooth(lat, std::abs(lat) == 90. ? 0. : p.lon().value()), 1e-9));
            }
        }
    }


    SECTION("to a coarser grid: the first input point picking an output point") {
        auto result = interpolate(*m, field(out, smooth), *in);
        EXPECT(!result.hasMissing());
    }


    SECTION("spec") {
        EXPECT_EQUAL(m->json_str(), "restructure");

        auto other = make("{type: restructure, nclosest: 2}");
        EXPECT(other->json_str().find(R"("nclosest":2)") != std::string::npos);
        EXPECT(*make(other->json_str()) == *other);
    }
}


}  // namespace mir::interpolation::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
