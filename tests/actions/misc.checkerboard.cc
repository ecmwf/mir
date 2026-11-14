// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <set>
#include <vector>

#include "action.h"


namespace mir::tests::actions {


CASE("misc.checkerboard") {
    const std::vector<std::string> grids{"{grid: [10, 10]}", "{grid: O16}", "{grid: H8}"};
    for (const std::string& grid : grids) {
        SECTION(grid) {
            auto in  = field(grid, [](double lat, double) { return lat; }, 2);
            auto out = perform(*make("misc.checkerboard"), in);

            EXPECT_EQUAL(out.dimensions(), 2);
            EXPECT_EQUAL(out.values(0).size(), in.values(0).size());
            EXPECT(!out.hasMissing());

            for (size_t which = 0; which < 2; ++which) {
                const auto& v = out.values(which);

                EXPECT(std::set<double>(v.begin(), v.end()) == std::set<double>({0., 1.}));
            }
        }
    }
}


CASE("misc.checkerboard, frequencies") {
    // four boxes (two in longitude, two in latitude), alternating
    auto in  = field("{grid: [10, 10]}", [](double lat, double) { return lat; });
    auto out = perform(*make("{type: misc.checkerboard, frequencies: [2, 2], 0-1: true}"), in);

    for (std::unique_ptr<repres::Iterator> it(out.representation()->iterator()); it->next();) {
        const auto& p  = it->pointUnrotated();
        const auto lat = p.lat().value();
        const auto lon = p.lon().normalise(Longitude::GREENWICH).value();

        if (lat > -90.) {
            const bool east  = lon >= 180.;
            const bool south = lat <= 0.;

            EXPECT(out.values(0)[it->index()] == (east != south ? 1. : 0.));
        }
    }
}


CASE("misc.checkerboard, with missing values") {
    SECTION("missing values are kept") {
        auto in  = field("{grid: [10, 10]}", [](double lat, double) { return lat > 50 ? 9999. : lat; }, 1, 9999.);
        auto out = perform(*make("misc.checkerboard"), in);

        EXPECT(out.hasMissing());
        EXPECT_EQUAL(count_missing(out), count_missing(in));

        for (std::unique_ptr<repres::Iterator> it(out.representation()->iterator()); it->next();) {
            const auto v = out.values(0)[it->index()];

            EXPECT(it->pointUnrotated().lat().value() > 50 ? v == 9999. : (v == 0. || v == 1.));
        }
    }

    SECTION("only missing values: unchanged") {
        auto in  = field("{grid: [10, 10]}", [](double, double) { return 9999.; }, 1, 9999.);
        auto out = perform(*make("misc.checkerboard"), in);

        EXPECT(out.values(0) == in.values(0));
    }
}


CASE("misc.checkerboard, properties") {
    auto a = make("{type: misc.checkerboard, frequencies: [2, 2], 0-1: true}");
    auto b = make(a->json_str(false));

    EXPECT_EQUAL(b->json_str(false), a->json_str(false));
    EXPECT_EQUAL(a->json_str(), R"({"type":"misc.checkerboard","0-1":true,"frequencies":[2,2]})");
}


}  // namespace mir::tests::actions


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
