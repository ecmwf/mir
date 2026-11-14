// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <vector>

#include "action.h"


namespace mir::tests::actions {


size_t number_of_values(size_t T) {
    return (T + 1) * (T + 2);
}


/// Position of the (real part of the) coefficient (m, n), triangular truncation T
size_t index(size_t T, size_t m, size_t n) {
    size_t i = 0;

    for (size_t k = 0; k < m; ++k) {
        i += T - k + 1;
    }

    return 2 * (i + n - m);
}


/// Spectral field, the coefficient values as their position
data::MIRField spectral(size_t T, size_t dimensions = 1) {
    MIRValuesVector values(number_of_values(T));

    for (size_t i = 0; i < values.size(); ++i) {
        values[i] = static_cast<double>(i + 1);
    }

    return field("{grid: T" + std::to_string(T) + "}", values, dimensions);
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


}  // namespace mir::tests::actions


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
