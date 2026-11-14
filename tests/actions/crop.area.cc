// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <vector>

#include "mir/util/BoundingBox.h"

#include "action.h"


namespace mir::tests::actions {


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


}  // namespace mir::tests::actions


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
