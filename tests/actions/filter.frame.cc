// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <vector>

#include "action.h"


namespace mir::tests::actions {


CASE("filter.frame") {
    struct test_t {
        std::string grid;
        size_t frame;
        size_t missing;
    };

    // O16 rows have 20, 24, ..., 80 (then symmetric) points, the interior excludes 2 rows/points on each side
    size_t O16 = 0;

    for (size_t j = 2; j < 16; ++j) {
        O16 += 2 * (20 + 4 * j - 4);
    }

    const std::vector<test_t> tests{
        test_t{"{grid: [10, 10]}", 2, (36 - 4) * (19 - 4)},
        test_t{"{grid: [10, 10]}", 1, (36 - 2) * (19 - 2)},
        test_t{"{grid: [10, 10], area: [60, 0, -30, 350]}", 3, (36 - 6) * (10 - 6)},
        test_t{"{grid: F16}", 2, (64 - 4) * (32 - 4)},
        test_t{"{grid: O16}", 2, O16},
    };
    for (const auto& test : tests) {
        SECTION(test.grid + " frame: " + std::to_string(test.frame)) {
            auto in  = field(test.grid, [](double lat, double) { return lat; }, 2);
            auto out = perform("{type: filter.frame, frame: " + std::to_string(test.frame) + "}", in);

            EXPECT_EQUAL(out.dimensions(), 2);
            EXPECT(out.hasMissing());
            EXPECT_EQUAL(count_missing(out, 0), test.missing);
            EXPECT_EQUAL(count_missing(out, 1), test.missing);

            // the frame keeps its values
            for (size_t i = 0; i < in.values(0).size(); ++i) {
                const auto v = out.values(0)[i];

                EXPECT(v == 9999. || v == in.values(0)[i]);
            }
        }
    }
}


CASE("filter.frame, no effect") {
    // a frame wider than the grid keeps all values
    auto in  = field("{grid: [10, 10]}", [](double lat, double) { return lat; });
    auto out = perform("{type: filter.frame, frame: 10}", in);

    EXPECT(!out.hasMissing());
    EXPECT(out.values(0) == in.values(0));
}


CASE("filter.frame, properties") {
    auto a = make("{type: filter.frame, frame: 2}");

    EXPECT_EQUAL(a->json_str(), R"({"type":"filter.frame","frame":2})");
    EXPECT(*a == *make("{type: filter.frame, frame: 2}"));
    EXPECT(*a != *make("{type: filter.frame, frame: 3}"));
}


CASE("filter.frame, errors") {
    EXPECT_THROWS(make("filter.frame"));  // frame is required

    // not supported
    EXPECT_THROWS(perform("{type: filter.frame, frame: 2}", field("{grid: H8}")));
}


}  // namespace mir::tests::actions


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
