// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

#include "mir/util/Angles.h"

#include "action.h"


namespace mir::tests::actions {


/// Expected pattern value, for the (non-missing) range of input values
double pattern(double lat, double lon, double min, double max, double f1, double f2) {
    return (max - min) * 0.5 * std::sin(f1 / 2. * util::degree_to_radian(lon)) *
               std::cos(f2 * util::degree_to_radian(lat)) +
           (min + max) / 2.;
}


CASE("misc.pattern") {
    struct test_t {
        std::string spec;
        bool normalise;
        double f1;
        double f2;
    };

    const std::vector<std::string> grids{"{grid: [10, 10]}", "{grid: O16}", "{grid: H8}"};
    for (const auto& grid : grids) {
        const std::vector<test_t> tests{
            test_t{"misc.pattern", false, 6., 3.},
            test_t{"{type: misc.pattern, 0-1: true}", true, 6., 3.},
            test_t{"{type: misc.pattern, frequencies: [2, 1]}", false, 2., 1.},
        };
        for (const auto& test : tests) {
            SECTION(std::string(grid) + " " + test.spec) {
                auto in  = field(grid, [](double lat, double) { return lat; }, 2);
                auto out = perform(*make(test.spec), in);

                const auto& v0 = in.values(0);
                const auto min = test.normalise ? 0. : *std::min_element(v0.begin(), v0.end());
                const auto max = test.normalise ? 1. : *std::max_element(v0.begin(), v0.end());

                EXPECT_EQUAL(out.dimensions(), 2);
                EXPECT(!out.hasMissing());

                for (std::unique_ptr<repres::Iterator> it(out.representation()->iterator()); it->next();) {
                    const auto& p = it->pointUnrotated();
                    const auto v  = pattern(p.lat().value(), p.lon().value(), min, max, test.f1, test.f2);

                    EXPECT(same(out.values(0)[it->index()], v));
                    EXPECT(same(out.values(1)[it->index()], v));
                }
            }
        }
    }
}


CASE("misc.pattern, with missing values") {
    SECTION("missing values are kept, range of non-missing values") {
        auto in  = field("{grid: [10, 10]}", [](double lat, double) { return lat > 50 ? 9999. : lat; }, 1, 9999.);
        auto out = perform(*make("misc.pattern"), in);

        EXPECT(out.hasMissing());
        EXPECT_EQUAL(count_missing(out), count_missing(in));

        for (std::unique_ptr<repres::Iterator> it(out.representation()->iterator()); it->next();) {
            const auto& p = it->pointUnrotated();
            const auto v  = out.values(0)[it->index()];

            EXPECT(p.lat().value() > 50 ? v == 9999.
                                        : same(v, pattern(p.lat().value(), p.lon().value(), -90., 50., 6., 3.)));
        }
    }

    SECTION("only missing values: unchanged") {
        auto in  = field("{grid: [10, 10]}", [](double, double) { return 9999.; }, 2, 9999.);
        auto out = perform(*make("misc.pattern"), in);

        EXPECT(out.values(0) == in.values(0));
        EXPECT(out.values(1) == in.values(1));
    }
}


CASE("misc.pattern, properties") {
    auto a = make("{type: misc.pattern, frequencies: [2, 1], 0-1: true}");
    auto b = make(a->json_str(false));

    EXPECT_EQUAL(b->json_str(false), a->json_str(false));
    EXPECT_EQUAL(a->json_str(), R"({"type":"misc.pattern","0-1":true,"frequencies":[2,1]})");
}


}  // namespace mir::tests::actions


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
