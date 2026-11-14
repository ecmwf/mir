// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <vector>

#include "action.h"


namespace mir::tests::actions {


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


}  // namespace mir::tests::actions


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
