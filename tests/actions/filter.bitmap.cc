// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <vector>

#include "action.h"


namespace mir::tests::actions {


// ICE.1 is 17 rows (north to south) by 43 columns, rows 8 to 12 off for columns 12 to 32
const std::string ICE{"{grid: [1, 1], area: [16, 0, 0, 42]}"};


CASE("filter.bitmap") {
    auto in = field(ICE, [](double lat, double lon) { return lat + lon; }, 2, 9999.);

    EXPECT_EQUAL(in.values(0).size(), 17 * 43);
    EXPECT(!in.hasMissing());

    auto out = perform("{type: filter.bitmap, bitmap: ICE.1}", in);

    EXPECT_EQUAL(out.dimensions(), 2);
    EXPECT(out.hasMissing());

    for (size_t which = 0; which < 2; ++which) {
        EXPECT_EQUAL(count_missing(out, which), 5 * 21);

        for (std::unique_ptr<repres::Iterator> it(out.representation()->iterator()); it->next();) {
            const auto& p  = it->pointUnrotated();
            const auto row = static_cast<size_t>(16. - p.lat().value()) + 1;
            const auto col = static_cast<size_t>(p.lon().value()) + 1;
            const auto v   = out.values(which)[it->index()];

            const bool off = 8 <= row && row <= 12 && 12 <= col && col <= 32;

            EXPECT(off ? v == 9999. : same(v, p.lat().value() + p.lon().value()));
        }
    }
}


CASE("filter.bitmap, smaller field") {
    // fewer values than the bitmap: masking follows the bitmap rows, up to the number of values
    auto in  = field("{grid: [1, 1], area: [16, 0, 8, 42]}", [](double lat, double) { return lat; });
    auto out = perform("{type: filter.bitmap, bitmap: ICE.1}", in);

    EXPECT_EQUAL(in.values(0).size(), 9 * 43);
    EXPECT_EQUAL(count_missing(out), 2 * 21);
}


CASE("filter.bitmap, other bitmaps") {
    struct test_t {
        std::string bitmap;
        std::string grid;
    };

    const std::vector<test_t> tests{
        test_t{"GL05", "{grid: [1, 1], area: [52, 0, 0, 140]}"},
        test_t{"GRE01.MAP", "{grid: [1, 1], area: [80, 0, 0, 110]}"},
    };
    for (const auto& test : tests) {
        SECTION(test.bitmap) {
            auto in  = field(test.grid, [](double lat, double) { return lat; });
            auto out = perform("{type: filter.bitmap, bitmap: " + test.bitmap + "}", in);

            EXPECT(out.hasMissing());
            EXPECT(count_missing(out) > 0);
            EXPECT(count_missing(out) < in.values(0).size());
        }
    }
}


CASE("filter.bitmap, properties") {
    auto a = make("{type: filter.bitmap, bitmap: ICE.1}");

    EXPECT_EQUAL(a->json_str(), R"({"type":"filter.bitmap","bitmap":"ICE.1"})");
    EXPECT(*a == *make("{type: filter.bitmap, bitmap: ICE.1}"));
    EXPECT(*a != *make("{type: filter.bitmap, bitmap: GL05}"));
}


CASE("filter.bitmap, errors") {
    EXPECT_THROWS(make("filter.bitmap"));  // bitmap is required

    // more values than the bitmap
    EXPECT_THROWS(perform("{type: filter.bitmap, bitmap: ICE.1}", field("{grid: [1, 1]}")));

    EXPECT_THROWS(perform("{type: filter.bitmap, bitmap: does-not-exist}", field(ICE)));
}


}  // namespace mir::tests::actions


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
