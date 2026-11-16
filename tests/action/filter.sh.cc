// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <vector>

#include "action.h"


namespace mir::action::tests {


size_t number_of_values(size_t T) {
    return (T + 1) * (T + 2);
}


/// Spectral field, the coefficient values as their position
data::MIRField spectral(size_t T, size_t dimensions = 1) {
    MIRValuesVector values(number_of_values(T));

    for (size_t i = 0; i < values.size(); ++i) {
        values[i] = static_cast<double>(i + 1);
    }

    return field("{grid: T" + std::to_string(T) + "}", values, dimensions);
}


CASE("filter.sh-bandpass") {
    struct test_t {
        std::string bandpass;
        size_t minM;
        size_t maxM;
        size_t minN;
        size_t maxN;
    };

    const std::vector<test_t> tests{
        test_t{"[0, 3, 2, 8]", 0, 3, 2, 8},
        test_t{"[2, 2, 0, 10]", 2, 2, 0, 10},
        test_t{"[0, 10, 0, 10]", 0, 10, 0, 10},
        test_t{"[0, 0, 0, 0]", 0, 0, 0, 0},
    };
    for (const auto& test : tests) {
        SECTION(test.bandpass) {
            auto in  = spectral(10, 2);
            auto out = perform("{type: filter.sh-bandpass, bandpass: " + test.bandpass + ", truncation: 10}", in);

            EXPECT_EQUAL(out.representation()->truncation(), 10);
            EXPECT_EQUAL(out.dimensions(), 2);

            for (size_t which = 0; which < 2; ++which) {
                const auto& v = out.values(which);

                EXPECT_EQUAL(v.size(), in.values(which).size());

                // coefficients in the band are kept, others are zero
                for (size_t m = 0, i = 0; m <= 10; ++m) {
                    for (size_t n = m; n <= 10; ++n, i += 2) {
                        const bool band = test.minM <= m && m <= test.maxM && test.minN <= n && n <= test.maxN;

                        EXPECT(v[i] == (band ? in.values(which)[i] : 0.));
                        EXPECT(v[i + 1] == (band ? in.values(which)[i + 1] : 0.));
                    }
                }
            }
        }
    }
}


CASE("filter.sh-bandpass, properties") {
    auto a = make_checked("{type: filter.sh-bandpass, bandpass: [0, 3, 2, 8], truncation: 10}");

    EXPECT_EQUAL(a->json_str(), R"({"type":"filter.sh-bandpass","bandpass":[0,3,2,8],"truncation":10})");
    EXPECT(*a != *make("{type: filter.sh-bandpass, bandpass: [0, 3, 2, 9], truncation: 10}"));
    EXPECT(*a != *make("{type: filter.sh-bandpass, bandpass: [1, 3, 2, 8], truncation: 10}"));
}


CASE("filter.sh-bandpass, errors") {
    EXPECT_THROWS(make("filter.sh-bandpass"));                                  // bandpass/truncation are required
    EXPECT_THROWS(make("{type: filter.sh-bandpass, bandpass: [0, 3, 2, 8]}"));  // truncation is required
    EXPECT_THROWS(make("{type: filter.sh-bandpass, truncation: 10}"));          // bandpass is required
    EXPECT_THROWS(make("{type: filter.sh-bandpass, bandpass: [0, 3, 2], truncation: 10}"));
    EXPECT_THROWS(make("{type: filter.sh-bandpass, bandpass: [4, 3, 2, 8], truncation: 10}"));
    EXPECT_THROWS(make("{type: filter.sh-bandpass, bandpass: [0, 3, 9, 8], truncation: 10}"));
    EXPECT_THROWS(make("{type: filter.sh-bandpass, bandpass: [0, 11, 2, 8], truncation: 10}"));
    EXPECT_THROWS(make("{type: filter.sh-bandpass, bandpass: [0, 3, 2, 11], truncation: 10}"));

    // truncation different than the input's
    EXPECT_THROWS(perform("{type: filter.sh-bandpass, bandpass: [0, 3, 2, 8], truncation: 10}", spectral(5)));
}


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


/// Position of the (real part of the) coefficient (m, n), triangular truncation T
size_t index(size_t T, size_t m, size_t n) {
    size_t i = 0;

    for (size_t k = 0; k < m; ++k) {
        i += T - k + 1;
    }

    return 2 * (i + n - m);
}


CASE("filter.sh-truncate") {
    struct test_t {
        size_t from;
        size_t to;
    };

    const std::vector<test_t> tests{test_t{10, 5}, test_t{10, 1}, test_t{5, 10}, test_t{1, 3}};
    for (const auto& test : tests) {
        SECTION("T" + std::to_string(test.from) + " to T" + std::to_string(test.to)) {
            auto in  = spectral(test.from, 2);
            auto out = perform("{type: filter.sh-truncate, truncation: " + std::to_string(test.to) + "}", in);

            EXPECT_EQUAL(out.representation()->truncation(), test.to);
            EXPECT_EQUAL(out.dimensions(), 2);

            for (size_t which = 0; which < 2; ++which) {
                const auto& v = out.values(which);

                EXPECT_EQUAL(v.size(), number_of_values(test.to));

                // coefficients are kept up to the lower truncation, or zero
                for (size_t m = 0; m <= test.to; ++m) {
                    for (size_t n = m; n <= test.to; ++n) {
                        const auto i = index(test.to, m, n);

                        if (n <= test.from) {
                            const auto j = index(test.from, m, n);

                            EXPECT(v[i] == in.values(which)[j]);
                            EXPECT(v[i + 1] == in.values(which)[j + 1]);
                        }
                        else {
                            EXPECT(v[i] == 0.);
                            EXPECT(v[i + 1] == 0.);
                        }
                    }
                }
            }
        }
    }

    SECTION("same truncation: unchanged") {
        auto in  = spectral(10);
        auto out = perform("{type: filter.sh-truncate, truncation: 10}", in);

        EXPECT_EQUAL(out.representation()->truncation(), 10);
        EXPECT(out.values(0) == in.values(0));
    }
}


CASE("filter.sh-truncate, properties") {
    auto a = make("{type: filter.sh-truncate, truncation: 10}");

    EXPECT_EQUAL(a->json_str(), R"({"type":"filter.sh-truncate","truncation":10})");
    EXPECT(*a == *make("{type: filter.sh-truncate, truncation: 10}"));
    EXPECT(*a != *make("{type: filter.sh-truncate, truncation: 11}"));
}


CASE("filter.sh-truncate, errors") {
    EXPECT_THROWS(make("filter.sh-truncate"));  // truncation is required
    EXPECT_THROWS(make("{type: filter.sh-truncate, truncation: 0}"));

    // gridded input
    EXPECT_THROWS(perform("{type: filter.sh-truncate, truncation: 10}", field("{grid: [10, 10]}")));
}


CASE("filter.sh-*, printing") {
    EXPECT_EQUAL(str(*make("{type: filter.sh-bandpass, bandpass: [0, 3, 2, 8], truncation: 10}")),
                 "Bandpass[M=0/3,N=2/8]");
    EXPECT_EQUAL(str(*make("{type: filter.sh-cesaro-summation-filter, cesaro-k: 2.5, cesaro-truncation: 5}")),
                 "CesaroSummationFilter[k=2.5,truncation=5]");
    EXPECT(str(*make("{type: filter.sh-truncate, truncation: 10}")).find("10") != std::string::npos);
}


}  // namespace mir::action::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
