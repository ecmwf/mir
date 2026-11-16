// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <vector>

#include "mir/util/BoundingBox.h"

#include "action.h"


namespace mir::action::tests {


// first, so the areas are not cropped/masked before in this process (cropping and masking are cached in memory)
CASE("crop.area, mask.area, without caching") {
    struct test_t {
        std::string grid;
        std::string area;
        util::BoundingBox bbox;  // of the cropped representation
        size_t numberOfPoints;
    };

    const std::vector<test_t> tests{
        test_t{"{grid: [1, 1]}", "[20, -20, -20, 20]", {20, -20, -20, 20}, 41 * 41},
        test_t{"{grid: [1, 1]}", "[20, 340, -20, 380]", {20, 340, -20, 380}, 41 * 41},
        test_t{"{grid: [1, 1], rotation: [-40, 22]}", "[20, -20, -20, 20]", {20, -20, -20, 20}, 41 * 41},
    };
    for (const auto& test : tests) {
        SECTION(test.grid + " crop.area " + test.area) {
            auto in = field(test.grid, [](double lat, double lon) { return lat + lon; }, 2);

            auto out = perform("{type: crop.area, area: " + test.area + ", caching: false}", in);
            auto ref = perform("{type: crop.area, area: " + test.area + "}", in);

            EXPECT_EQUAL(out.dimensions(), 2);
            EXPECT_EQUAL(out.values(0).size(), test.numberOfPoints);
            EXPECT(out.representation()->boundingBox() == test.bbox);

            // the same as cached
            EXPECT(out.values(0) == ref.values(0));
            EXPECT(out.values(1) == ref.values(1));
            EXPECT(out.representation()->sameAs(*ref.representation()));
        }
    }

    SECTION("{grid: O32} crop.area [45, -5, 25, 35]") {
        auto in  = field("{grid: O32}", [](double lat, double) { return lat; });
        auto out = perform("{type: crop.area, area: [45, -5, 25, 35], caching: false}", in);

        EXPECT(0 < out.values(0).size() && out.values(0).size() < in.values(0).size());

        // points from north to south
        double north = 90.;
        for (std::unique_ptr<repres::Iterator> it(out.representation()->iterator()); it->next();) {
            const auto lat = it->pointUnrotated().lat().value();

            EXPECT(25. <= lat && lat <= 45.);
            EXPECT(lat <= north);
            EXPECT(same(out.values(0)[it->index()], lat));

            north = lat;
        }
    }

    SECTION("points, not from north to south: cropping sorts them") {
        auto in = perform(*make("{type: interpolate.grid2points, latitudes: [-5, 0, 5, 8], longitudes: [-5, 0, 5, 8]}"),
                          field("{grid: [1, 1]}", [](double lat, double) { return lat; }));
        auto out = perform(*make("{type: crop.area, area: [10, -10, -10, 10], caching: false}"), in);

        EXPECT(out.values(0) == MIRValuesVector({8, 5, 0, -5}));
    }

    SECTION("{grid: [1, 1]} mask.area [50, -20, 20, 30]") {
        auto in  = field("{grid: [1, 1]}", [](double lat, double) { return lat; }, 1, 9999.);
        auto out = perform("{type: mask.area, area: [50, -20, 20, 30], caching: false}", in);
        auto ref = perform("{type: mask.area, area: [50, -20, 20, 30]}", in);

        EXPECT_EQUAL(out.values(0).size(), in.values(0).size());
        EXPECT_EQUAL(out.values(0).size() - count_missing(out), 31 * 51);
        EXPECT(out.values(0) == ref.values(0));
    }
}


CASE("crop.area") {
    struct test_t {
        std::string grid;
        std::string area;
        size_t numberOfPoints;
    };

    const std::vector<test_t> tests{
        test_t{"{grid: [1, 1]}", "[10, -10, -10, 10]", 21 * 21},
        test_t{"{grid: [1, 1]}", "[10, 350, -10, 370]", 21 * 21},  // across the date line
        test_t{"{grid: [1, 1]}", "[0.5, -0.5, -0.5, 0.5]", 1},     // single point
        test_t{"{grid: O32}", "[60, -10, 30, 40]", 0},
        test_t{"{grid: [1, 1], rotation: [-40, 22]}", "[10, -10, -10, 10]", 21 * 21},
    };
    for (const auto& test : tests) {
        SECTION(test.grid + " " + test.area) {
            auto in  = field(test.grid, [](double lat, double lon) { return lat; }, 2);
            auto out = perform("{type: crop.area, area: " + test.area + "}", in);

            EXPECT_EQUAL(out.dimensions(), 2);
            EXPECT(out.values(0).size() <= in.values(0).size());
            EXPECT(out.values(0).size() > 0);

            if (test.numberOfPoints > 0) {
                EXPECT_EQUAL(out.values(0).size(), test.numberOfPoints);
            }

            // cropped values are of points in the area
            const auto bbox = make("{type: crop.area, area: " + test.area + "}")->outputBoundingBox();

            for (std::unique_ptr<repres::Iterator> it(out.representation()->iterator()); it->next();) {
                EXPECT(bbox.contains(it->pointUnrotated()));
            }
        }
    }
}


CASE("crop.area, with missing values") {
    auto in  = field("{grid: [1, 1]}", [](double lat, double) { return lat > 0 ? 9999. : lat; }, 1, 9999.);
    auto out = perform("{type: crop.area, area: [10, -10, -10, 10]}", in);

    EXPECT(out.hasMissing());
    EXPECT_EQUAL(count_missing(out), 10 * 21);
}


CASE("crop.area, properties") {
    auto a = make("{type: crop.area, area: [10, -10, -10, 10]}");

    EXPECT(a->isCropAction());
    EXPECT(a->canCrop());
    EXPECT(a->outputBoundingBox() == util::BoundingBox(10, -10, -10, 10));

    EXPECT(*a == *make("{type: crop.area, area: 10/-10/-10/10}"));
    EXPECT(*a != *make("{type: crop.area, area: [10, -10, -10, 11]}"));
    EXPECT(*a != *make("{type: mask.area, area: [10, -10, -10, 10]}"));
}


CASE("crop.area, errors") {
    EXPECT_THROWS(make("crop.area"));  // area is required

    // empty area
    EXPECT_THROWS(perform("{type: crop.area, area: [0.6, 0.2, 0.4, 0.8]}", field("{grid: [1, 1]}")));

    // representation without cropping support
    EXPECT_THROWS(perform("{type: crop.area, area: [60, -10, 30, 40]}", field("{grid: H8}")));
}


CASE("mask.area") {
    const std::vector<std::string> grids{"{grid: [1, 1]}", "{grid: O32}", "{grid: H8}"};
    for (const std::string& grid : grids) {
        SECTION(grid) {
            auto in  = field(grid, [](double lat, double) { return lat; }, 2, 9999.);
            auto out = perform("{type: mask.area, area: [60, -10, 30, 40]}", in);

            // same points, missing values outside the area
            EXPECT_EQUAL(out.dimensions(), 2);
            EXPECT_EQUAL(out.values(0).size(), in.values(0).size());
            EXPECT(out.hasMissing());

            for (std::unique_ptr<repres::Iterator> it(out.representation()->iterator()); it->next();) {
                const auto lat = it->pointUnrotated().lat().value();
                const auto v   = out.values(1)[it->index()];

                EXPECT(v == 9999. || (30. - 1e-9 <= lat && lat <= 60. + 1e-9 && same(v, lat)));
            }
        }
    }
}


CASE("mask.area, properties") {
    auto a = make("{type: mask.area, area: [60, -10, 30, 40]}");

    EXPECT(!a->isCropAction());
    EXPECT(*a == *make("{type: mask.area, area: 60/-10/30/40}"));
    EXPECT(*a != *make("{type: mask.area, area: [60, -10, 30, 41]}"));
}


CASE("mask.area, errors") {
    EXPECT_THROWS(make("mask.area"));  // area is required
    EXPECT_THROWS(perform("{type: mask.area, area: [0.6, 0.2, 0.4, 0.8]}", field("{grid: [1, 1]}")));
}


CASE("crop.area, mask.area, printing") {
    auto crop = make("{type: crop.area, area: [10, -10, -10, 10]}");
    auto mask = make("{type: mask.area, area: [10, -10, -10, 10]}");

    EXPECT(str(*crop).rfind("AreaCropper[bbox=", 0) == 0);
    EXPECT(str(*mask).rfind("AreaMasker[bbox=", 0) == 0);

    // masking keeps all points, its output bounding box is the area
    EXPECT(mask->outputBoundingBox() == util::BoundingBox(10, -10, -10, 10));
    EXPECT(!mask->canCrop());
}


}  // namespace mir::action::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
