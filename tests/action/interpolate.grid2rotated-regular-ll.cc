// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <sstream>
#include <vector>

#include "interpolate.h"


namespace mir::action::tests {


const std::string ROTATION = "rotation: [-40, 22]";


CASE("interpolate.grid2rotated-regular-ll") {
    struct test_t {
        std::string options;
        size_t numberOfPoints;
        util::BoundingBox bbox;  // in the rotated frame
    };

    const std::vector<test_t> tests{
        test_t{"grid: [10, 10]", 19 * 36, {90, 0, -90, 350}},
        test_t{"grid: [20, 20]", 9 * 18, {80, 0, -80, 340}},
        test_t{"grid: [2, 2], area: [10, -10, -9, 10]", 90 * 180, {89, 0, -89, 358}},  // shifted
        test_t{"grid: [2, 2], cropping: [10, -10, -10, 10]", 11 * 11, {10, -10, -10, 10}},
    };
    for (const auto& test : tests) {
        SECTION(test.options) {
            auto a =
                make_checked("{type: interpolate.grid2rotated-regular-ll, " + test.options + ", " + ROTATION + "}");
            auto out = perform(*a, field("{grid: O16}", LATITUDE, 2));

            EXPECT_EQUAL(out.dimensions(), 2);
            EXPECT_EQUAL(out.values(0).size(), test.numberOfPoints);
            EXPECT_EQUAL(out.representation()->numberOfPoints(), test.numberOfPoints);
            EXPECT(out.representation()->boundingBox() == test.bbox);
            EXPECT(out.representation()->uniqueName().find("-rot:-40:22:0") != std::string::npos);
            EXPECT(!out.hasMissing());
            EXPECT(contained(out, a->outputBoundingBox()));
        }
    }

    SECTION("rotated (south) pole") {
        auto out = perform("{type: interpolate.grid2rotated-regular-ll, grid: [10, 10], " + ROTATION + "}",
                           field("{grid: O16}", LATITUDE));

        size_t n = 0;

        const auto& rotated   = out.representation()->canonicalGrid();
        const auto& unrotated = out.representation()->canonicalGridUnrotated();

        for (auto it = rotated.cbegin(), jt = unrotated.cbegin(); it; ++it, ++jt) {
            if (Latitude{std::get<PointLonLat>(*jt).lat()} == Latitude::SOUTH_POLE) {
                const auto p = std::get<PointLonLat>(*it);

                EXPECT(same(p.lat(), -40., 1e-6));
                EXPECT(same(p.lon(), 22., 1e-6));
                EXPECT(std::abs(out.values(0)[it.index()] - -40.) < 0.2);

                ++n;
            }
        }

        EXPECT(n > 0);
    }
}


CASE("interpolate.grid2rotated-regular-ll, values") {
    check_values(*make_checked("{type: interpolate.grid2rotated-regular-ll, grid: [10, 10], " + ROTATION + "}"));
    check_values(*make_checked("{type: interpolate.grid2rotated-regular-ll, grid: [10, 10], " + ROTATION +
                               ", interpolation: nn}"),
                 "{grid: O16}", 3.5);
}


CASE("interpolate.grid2rotated-regular-ll, cropping") {
    auto a = make_mutable("{type: interpolate.grid2rotated-regular-ll, grid: [2, 2], " + ROTATION + "}");

    EXPECT(!a->canCrop());
    EXPECT(a->outputBoundingBox() == util::BoundingBox(90, 0, -90, 360));

    // cropping in the rotated frame, bounding box (unrotated) containing it
    EXPECT(a->mergeWithNext(*make("{type: crop.area, area: [10, -10, -10, 10]}")));
    EXPECT(a->canCrop());

    const auto bbox = a->outputBoundingBox();

    EXPECT(!(bbox == util::BoundingBox(10, -10, -10, 10)));
    EXPECT(util::BoundingBox(61, 2, 39, 42).contains(bbox));
    EXPECT(bbox.contains(util::BoundingBox(59, 3, 40, 41)));

    auto b = make("{type: interpolate.grid2rotated-regular-ll, grid: [2, 2], " + ROTATION +
                  ", cropping: [10, -10, -10, 10]}");
    EXPECT(*a == *b);
    EXPECT(b->outputBoundingBox() == bbox);

    auto out = perform(*a, field("{grid: O16}", LATITUDE));

    EXPECT_EQUAL(out.values(0).size(), 11 * 11);
    EXPECT(contained(out, bbox));
    EXPECT(max_error(out, LATITUDE) < 0.2);
}


CASE("interpolate.grid2rotated-regular-ll, properties") {
    auto a = make("{type: interpolate.grid2rotated-regular-ll, grid: [2, 2], " + ROTATION + "}");

    EXPECT(a->isRegridAction());
    EXPECT(!a->canCrop());

    std::string name;

    EXPECT(!a->getGriddedTargetName(name));

    EXPECT_EQUAL(a->type(), "interpolate.grid2rotated-regular-ll");
    EXPECT_EQUAL(std::string(a->name()), "Gridded2RotatedLL");

    const auto json = a->json_str();

    EXPECT(json.find(R"("rotation":[-40,22],"grid":[2,2],"area":[90,0,-90,358])") != std::string::npos);

    std::ostringstream print;
    print << *a;

    EXPECT(print.str().find("Gridded2RotatedLL[increments=") == 0);
    EXPECT(print.str().find(",rotation=") != std::string::npos);
    EXPECT(print.str().find("interpolation=linear") != std::string::npos);
}


CASE("interpolate.grid2rotated-regular-ll, comparison") {
    auto a = make("{type: interpolate.grid2rotated-regular-ll, grid: [2, 2], " + ROTATION + "}");

    EXPECT(*a == *make("{type: interpolate.grid2rotated-regular-ll, grid: 2/2, rotation: -40/22}"));
    EXPECT(*a ==
           *make("{type: interpolate.grid2rotated-regular-ll, grid: [2, 2], " + ROTATION + ", interpolation: linear}"));

    const std::vector<std::string> others{
        "{type: interpolate.grid2rotated-regular-ll, grid: [2, 2], rotation: [-40, 23]}",
        "{type: interpolate.grid2rotated-regular-ll, grid: [2, 2], rotation: [-41, 22]}",
        "{type: interpolate.grid2rotated-regular-ll, grid: [1, 1], rotation: [-40, 22]}",
        "{type: interpolate.grid2rotated-regular-ll, grid: [2, 2], rotation: [-40, 22], area: [10, -10, -9, 10]}",
        "{type: interpolate.grid2rotated-regular-ll, grid: [2, 2], rotation: [-40, 22], interpolation: nn}",
        "{type: interpolate.grid2regular-ll, grid: [2, 2]}",
        "{type: interpolate.grid2rotated-regular-gg, regular: 45, rotation: [-40, 22]}",
    };
    for (const std::string& other : others) {
        EXPECT(*a != *make(other));
    }
}


CASE("interpolate.grid2rotated-regular-ll, errors") {
    EXPECT_THROWS(make("{type: interpolate.grid2rotated-regular-ll, " + ROTATION + "}"));  // grid is required
    EXPECT_THROWS(make("{type: interpolate.grid2rotated-regular-ll, grid: [2, 2]}"));      // rotation is required
    EXPECT_THROWS(make("{type: interpolate.grid2rotated-regular-ll, grid: [2, 2, 2], " + ROTATION + "}"));
    EXPECT_THROWS(make("{type: interpolate.grid2rotated-regular-ll, grid: [2, 2], rotation: [-40, 22, 0]}"));
}


}  // namespace mir::action::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
