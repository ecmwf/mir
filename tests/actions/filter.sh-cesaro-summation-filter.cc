// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <vector>

#include "action.h"


namespace mir::tests::actions {


/// Cesàro filter factor of total wavenumber n, for truncation T, order k and starting wavenumber Tmin
double cesaro(size_t n, size_t T, double k, size_t Tmin) {
    double f = 1.;

    for (size_t j = Tmin; j <= n; ++j) {
        const auto a = static_cast<double>(T - j + 1);
        f *= a / (a + k);
    }

    return f;
}


CASE("filter.sh-cesaro-summation-filter") {
    struct test_t {
        std::string spec;
        double k;
        size_t Tmin;
    };

    const size_t T = 10;
    MIRValuesVector values((T + 1) * (T + 2), 1.);
    auto in = field("{grid: T10}", values, 2);

    const std::vector<test_t> tests{
        test_t{"filter.sh-cesaro-summation-filter", 2., 1},
        test_t{"{type: filter.sh-cesaro-summation-filter, cesaro_k: 2.5, cesaro_truncation: 5}", 2.5, 5},
        test_t{"{type: filter.sh-cesaro-summation-filter, cesaro-k: 0, cesaro-truncation: 1}", 0., 1},
        test_t{"{type: filter.sh-cesaro-summation-filter, cesaro-truncation: 10}", 2., 10},
    };
    for (const auto& test : tests) {
        SECTION(test.spec) {
            auto out = perform(test.spec, in);

            EXPECT_EQUAL(out.representation()->truncation(), T);
            EXPECT_EQUAL(out.dimensions(), 2);

            for (size_t which = 0; which < 2; ++which) {
                const auto& v = out.values(which);

                EXPECT_EQUAL(v.size(), values.size());

                // filtered by total wavenumber, unfiltered below the starting wavenumber
                for (size_t m = 0, i = 0; m <= T; ++m) {
                    for (size_t n = m; n <= T; ++n, i += 2) {
                        const auto f = cesaro(n, T, test.k, test.Tmin);

                        EXPECT(n < test.Tmin ? f == 1. : f <= 1.);
                        EXPECT(same(v[i], f));
                        EXPECT(same(v[i + 1], f));
                    }
                }
            }
        }
    }
}


CASE("filter.sh-cesaro-summation-filter, properties") {
    auto a = make_checked("{type: filter.sh-cesaro-summation-filter, cesaro_k: 2.5, cesaro_truncation: 5}");

    EXPECT_EQUAL(a->json_str(), R"({"type":"filter.sh-cesaro-summation-filter","cesaro-k":2.5,"cesaro-truncation":5})");

    EXPECT(*make("filter.sh-cesaro-summation-filter") ==
           *make("{type: filter.sh-cesaro-summation-filter, cesaro-k: 2, cesaro-truncation: 1}"));
    EXPECT(*a != *make("{type: filter.sh-cesaro-summation-filter, cesaro-k: 2, cesaro-truncation: 5}"));
    EXPECT(*a != *make("{type: filter.sh-cesaro-summation-filter, cesaro-k: 2.5, cesaro-truncation: 6}"));
}


CASE("filter.sh-cesaro-summation-filter, errors") {
    EXPECT_THROWS(make("{type: filter.sh-cesaro-summation-filter, cesaro-k: -1}"));
    EXPECT_THROWS(make("{type: filter.sh-cesaro-summation-filter, cesaro-truncation: 0}"));

    // starting wavenumber above the input truncation
    MIRValuesVector values(6 * 7, 1.);

    EXPECT_THROWS(
        perform("{type: filter.sh-cesaro-summation-filter, cesaro-truncation: 6}", field("{grid: T5}", values)));
}


}  // namespace mir::tests::actions


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
