// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <string>
#include <vector>

#include "mir/util/BoundingBox.h"

#include "transform.h"


namespace mir::action::tests {


// few points, as transforming to unstructured grids is slow
const std::string UNSTRUCTURED =
    "{type: unstructured_ll, latitudes: [89.5, 60, 30, 0, -45, -89.5], longitudes: [0, 45, 90, 180, 270, 359.5]}";


CASE("transform.sh-scalar-to-gridspec") {
    struct test_t {
        std::string grid;
        size_t numberOfPoints;
    };

    const std::vector<test_t> tests{
        test_t{"{grid: O16}", 1600}, test_t{"{grid: F16}", 2048}, test_t{"{grid: [30, 30]}", 7 * 12},
        test_t{"{grid: O16, rotation: [-40, 22]}", 1600},  // values at the geographic points
    };
    for (const auto& test : tests) {
        SECTION(test.grid) {
            const auto spec = "{type: transform.sh-scalar-to-gridspec, grid: " + test.grid + "}";
            check_scalar(spec, test.numberOfPoints);
            check_scalar(spec, test.numberOfPoints, 2);
            check_representation(spec, test.grid);
        }
    }

    SECTION("unstructured") {
        const auto spec = "{type: transform.sh-scalar-to-gridspec, grid: " + UNSTRUCTURED + "}";
        check_scalar(spec, 6);
        check_scalar(spec, 6, 2);
    }

    // regional Gaussian grids: transform to the global grid, then crop
    const std::vector<std::string> grids{"{grid: O16, area: [60, -10, 30, 40]}",
                                         "{grid: F16, area: [60, -10, 30, 40]}"};
    for (const std::string& grid : grids) {
        SECTION(grid) {
            // cropping to the regional grid (Gaussian latitudes), not to the area
            auto a = make("{type: transform.sh-scalar-to-gridspec, grid: " + grid + "}");

            EXPECT(contains(a->json_str(false), R"("cropping":[58.143,)"));
            EXPECT(contains(a->json_str(false), R"("grid":{"grid":")"));

            const util::BoundingBox bbox(60, -10, 30, 40);
            auto out = perform(*a, sh(coefficients({{index(0, 1), 1.}})));

            EXPECT(0 < out.values(0).size() && out.values(0).size() < 1600);

            for (const auto& point : out.representation()->canonicalGridUnrotated()) {
                EXPECT(bbox.contains(std::get<PointLonLat>(point)));
            }

            for_each(out, 0, [](double v, double lat, double) { EXPECT(same(v, P10(lat))); });
        }
    }
}


CASE("transform.sh-vod-to-uv-gridspec") {
    check_vod_to_uv("{type: transform.sh-vod-to-uv-gridspec, grid: {grid: O16}}", 1600);
    check_vod_to_uv("{type: transform.sh-vod-to-uv-gridspec, grid: {grid: F16}}", 2048, 2);
    check_vod_to_uv("{type: transform.sh-vod-to-uv-gridspec, grid: {grid: O16, rotation: [-40, 22]}}", 1600);
    check_vod_to_uv("{type: transform.sh-vod-to-uv-gridspec, grid: " + UNSTRUCTURED + "}", 6);
    check_vod_to_uv("{type: transform.sh-vod-to-uv-gridspec, grid: " + UNSTRUCTURED + "}", 6, 2);
}


CASE("transform.sh-*-to-gridspec, properties") {
    auto a = make_checked("{type: transform.sh-scalar-to-gridspec, grid: {grid: O16}}");
    auto b = make_checked("{type: transform.sh-vod-to-uv-gridspec, grid: {grid: O16}}");

    EXPECT_EQUAL(std::string(a->name()), "ShToGridSpec");
    EXPECT(contains(str(*a), "ShToGridSpec[type=local") && contains(str(*a), ",gridspec="));
    EXPECT(contains(str(*b), "invtrans=<vod2uv>"));
    EXPECT(contains(a->json_str(false), R"("grid":{"grid":"O16"})"));

    std::string name;

    EXPECT(!a->getGriddedTargetName(name));

    EXPECT(*a == *make("{type: transform.sh-scalar-to-gridspec, grid: {grid: o16}}"));
    EXPECT(*a != *b);
    EXPECT(*a != *make("{type: transform.sh-scalar-to-gridspec, grid: {grid: O32}}"));
    EXPECT(*a != *make("{type: transform.sh-scalar-to-gridspec, grid: {grid: O16, area: [60, -10, 30, 40]}}"));
    EXPECT(*a != *make("{type: transform.sh-scalar-to-gridspec, grid: {grid: O16, rotation: [-40, 22]}}"));
    EXPECT(*a != *make("{type: transform.sh-scalar-to-namedgrid, grid: O16}"));

    auto c = make("{type: transform.sh-scalar-to-gridspec, grid: " + unstructured("weather-params.pts") + "}");

    EXPECT(*c == *make("{type: transform.sh-scalar-to-gridspec, grid: " + unstructured("weather-params.pts") + "}"));
    EXPECT(*c != *make("{type: transform.sh-scalar-to-gridspec, grid: " + unstructured("ICON.pts") + "}"));
    EXPECT(*c != *make("{type: interpolate.grid2gridspec, grid: " + unstructured("weather-params.pts") + "}"));
}


CASE("transform.sh-*-to-gridspec, errors") {
    EXPECT_THROWS(make("transform.sh-scalar-to-gridspec"));  // grid is required
    EXPECT_THROWS(make("{type: transform.sh-vod-to-uv-gridspec, grid: {grid: unknown}}"));
    EXPECT_THROWS(perform("{type: transform.sh-scalar-to-gridspec, grid: {grid: O16}}", field("{grid: O16}")));
}


}  // namespace mir::action::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
