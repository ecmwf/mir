// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <fstream>
#include <string>
#include <vector>

#include "eckit/filesystem/TmpFile.h"

#include "mir/util/BoundingBox.h"

#include "transform.h"


namespace mir::action::tests {


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

            for (std::unique_ptr<repres::Iterator> it(out.representation()->iterator()); it->next();) {
                EXPECT(bbox.contains(it->pointUnrotated()));
            }

            for_each(out, 0, [](double v, double lat, double) { EXPECT(same(v, P10(lat))); });
        }
    }
}


CASE("transform.sh-vod-to-uv-gridspec") {
    check_vod_to_uv("{type: transform.sh-vod-to-uv-gridspec, grid: {grid: O16}}", 1600);
    check_vod_to_uv("{type: transform.sh-vod-to-uv-gridspec, grid: {grid: F16}}", 2048, 2);
    check_vod_to_uv("{type: transform.sh-vod-to-uv-gridspec, grid: {grid: O16, rotation: [-40, 22]}}", 1600);
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
}


CASE("transform.sh-*-to-gridspec, errors") {
    EXPECT_THROWS(make("transform.sh-scalar-to-gridspec"));  // grid is required
    EXPECT_THROWS(make("{type: transform.sh-vod-to-uv-gridspec, grid: {grid: unknown}}"));
    EXPECT_THROWS(perform("{type: transform.sh-scalar-to-gridspec, grid: {grid: O16}}", field("{grid: O16}")));
}


size_t numberOfPoints(const std::string& grid) {
    return representation("{grid: " + grid + "}")->numberOfPoints();
}


CASE("transform.sh-scalar-to-namedgrid") {
    const std::vector<std::string> grids{"O16", "N16", "F16"};
    for (const std::string& grid : grids) {
        SECTION(grid) {
            const auto spec = "{type: transform.sh-scalar-to-namedgrid, grid: " + grid + "}";
            check_scalar(spec, numberOfPoints(grid));
            check_scalar(spec, numberOfPoints(grid), 2);
            check_representation(spec, "{grid: " + grid + "}");
        }
    }

    SECTION("30/30") {
        check_scalar("{type: transform.sh-scalar-to-namedgrid, grid: 30/30}", 7 * 12);
    }
}


CASE("transform.sh-vod-to-uv-namedgrid") {
    check_vod_to_uv("{type: transform.sh-vod-to-uv-namedgrid, grid: O16}", 1600);
    check_vod_to_uv("{type: transform.sh-vod-to-uv-namedgrid, grid: F16}", 2048, 2);
}


CASE("transform.sh-*-to-namedgrid, plan compression") {
    const std::string spec = "{type: transform.sh-scalar-to-namedgrid, grid: O16}";

    SECTION("followed by an interpolation to the same grid: merge") {
        EXPECT(make_mutable(spec)->mergeWithNext(*make("{type: interpolate.grid2namedgrid, grid: O16}")));
        EXPECT(make_mutable(spec)->mergeWithNext(*make("{type: interpolate.grid2octahedral-gg, octahedral: 16}")));
    }

    SECTION("followed by an interpolation to another grid: no merge") {
        auto a = make_mutable(spec);

        EXPECT(!a->mergeWithNext(*make("{type: interpolate.grid2namedgrid, grid: N16}")));
        EXPECT(*a == *make(spec));
    }
}


CASE("transform.sh-*-to-namedgrid, properties") {
    auto a = make_checked("{type: transform.sh-scalar-to-namedgrid, grid: O16}");
    auto b = make_checked("{type: transform.sh-vod-to-uv-namedgrid, grid: O16}");

    EXPECT_EQUAL(std::string(a->name()), "ShToNamedGrid");
    EXPECT(contains(str(*a), "ShToNamedGrid[type=local") && contains(str(*a), ",grid=O16]"));
    EXPECT(contains(str(*b), "invtrans=<vod2uv>"));
    EXPECT(contains(a->json_str(false), R"("grid":"O16")"));

    // the grid name, canonical
    const std::vector<std::string> specs{"{type: transform.sh-scalar-to-namedgrid, grid: O16}",
                                         "{type: transform.sh-scalar-to-namedgrid, grid: o16}"};
    for (const auto& spec : specs) {
        std::string name;

        EXPECT(make(spec)->getGriddedTargetName(name));
        EXPECT_EQUAL(name, "O16");
        EXPECT(*a == *make(spec));
    }

    EXPECT(*a != *b);
    EXPECT(*a != *make("{type: transform.sh-scalar-to-namedgrid, grid: N16}"));
    EXPECT(*a != *make("{type: transform.sh-scalar-to-octahedral-gg, octahedral: 16}"));
    EXPECT(*a != *make("{type: transform.sh-scalar-to-namedgrid, grid: O16, atlas_trans_flt: true}"));
}


CASE("transform.sh-*-to-namedgrid, errors") {
    EXPECT_THROWS(make("transform.sh-scalar-to-namedgrid"));  // grid is required
    EXPECT_THROWS(make("{type: transform.sh-vod-to-uv-namedgrid, grid: unknown}"));
    EXPECT_THROWS(perform("{type: transform.sh-scalar-to-namedgrid, grid: O16}", field("{grid: O16}")));
}


CASE("transform.sh-scalar-to-rotated-namedgrid") {
    // values at the geographic points, as the grid is rotated
    const std::vector<std::string> grids{"O8", "N16", "F8"};
    for (const std::string& grid : grids) {
        SECTION(grid) {
            const auto spec =
                "{type: transform.sh-scalar-to-rotated-namedgrid, grid: " + grid + ", rotation: [-40, 22]}";
            const auto n = representation("{grid: " + grid + "}")->numberOfPoints();
            check_scalar(spec, n, grid == "O8" ? 2 : 1);
            check_representation(spec, "{grid: " + grid + ", rotation: [-40, 22]}");
        }
    }
}


CASE("transform.sh-vod-to-uv-rotated-namedgrid") {
    // winds relative to the geographic (not the rotated) grid
    check_vod_to_uv("{type: transform.sh-vod-to-uv-rotated-namedgrid, grid: O8, rotation: [-40, 22]}", 544);
    check_vod_to_uv("{type: transform.sh-vod-to-uv-rotated-namedgrid, grid: O8, rotation: [-40, 22]}", 544, 2);
}


CASE("transform.sh-*-to-rotated-namedgrid, properties") {
    auto a = make_checked("{type: transform.sh-scalar-to-rotated-namedgrid, grid: O16, rotation: [-40, 22]}");
    auto b = make_checked("{type: transform.sh-vod-to-uv-rotated-namedgrid, grid: O16, rotation: [-40, 22]}");

    EXPECT_EQUAL(std::string(a->name()), "ShToRotatedNamedGrid");
    EXPECT(contains(str(*a), "ShToRotatedNamedGrid[type=local") && contains(str(*a), ",grid=O16,rotation="));
    EXPECT(contains(str(*b), "invtrans=<vod2uv>"));
    EXPECT(contains(a->json_str(false), R"("grid":"O16","rotation":[-40,22])"));

    std::string name;

    EXPECT(!a->getGriddedTargetName(name));

    EXPECT(*a == *make("{type: transform.sh-scalar-to-rotated-namedgrid, grid: O16, rotation: -40/22}"));
    EXPECT(*a != *b);
    EXPECT(*a != *make("{type: transform.sh-scalar-to-rotated-namedgrid, grid: N16, rotation: [-40, 22]}"));
    EXPECT(*a != *make("{type: transform.sh-scalar-to-rotated-namedgrid, grid: O16, rotation: [-40, 20]}"));
    EXPECT(*a != *make("{type: transform.sh-scalar-to-namedgrid, grid: O16}"));
}


CASE("transform.sh-*-to-rotated-namedgrid, errors") {
    EXPECT_THROWS(make("{type: transform.sh-scalar-to-rotated-namedgrid, rotation: [-40, 22]}"));
    EXPECT_THROWS(make("{type: transform.sh-vod-to-uv-rotated-namedgrid, grid: O16}"));
    EXPECT_THROWS(make("{type: transform.sh-scalar-to-rotated-namedgrid, grid: O16, rotation: [-40, 22, 0]}"));
    EXPECT_THROWS(perform("{type: transform.sh-scalar-to-rotated-namedgrid, grid: O16, rotation: [-40, 22]}",
                          field("{grid: O16}")));
}


// grids by name, as typed grids (gridType=...;key=value) don't build a representation without a parametrisation
CASE("transform.sh-scalar-to-typedgrid") {
    SECTION("O16") {
        check_scalar("{type: transform.sh-scalar-to-typedgrid, grid: O16}", 1600);
        check_scalar("{type: transform.sh-scalar-to-typedgrid, grid: O16}", 1600, 2);
        check_representation("{type: transform.sh-scalar-to-typedgrid, grid: O16}", "{grid: O16}");
    }

    SECTION("30/30") {
        check_scalar("{type: transform.sh-scalar-to-typedgrid, grid: 30/30}", 7 * 12);
    }
}


CASE("transform.sh-vod-to-uv-typedgrid") {
    check_vod_to_uv("{type: transform.sh-vod-to-uv-typedgrid, grid: O16}", 1600);
    check_vod_to_uv("{type: transform.sh-vod-to-uv-typedgrid, grid: O16}", 1600, 2);
}


CASE("transform.sh-*-to-typedgrid, properties") {
    auto a = make_checked("{type: transform.sh-scalar-to-typedgrid, grid: O16}");
    auto b = make_checked("{type: transform.sh-vod-to-uv-typedgrid, grid: O16}");

    EXPECT_EQUAL(std::string(a->name()), "ShToTypedGrid");
    EXPECT(contains(str(*a), "ShToTypedGrid[type=local") && contains(str(*a), ",grid=O16]"));
    EXPECT(contains(str(*b), "invtrans=<vod2uv>"));
    EXPECT(contains(a->json_str(false), R"("grid":"O16")"));

    std::string name;

    EXPECT(!a->getGriddedTargetName(name));

    EXPECT(*a != *b);
    EXPECT(*a != *make("{type: transform.sh-scalar-to-typedgrid, grid: N16}"));
    EXPECT(*a != *make("{type: transform.sh-scalar-to-namedgrid, grid: O16}"));
    EXPECT(*a != *make("{type: transform.sh-scalar-to-typedgrid, grid: O16, atlas_trans_flt: true}"));
}


CASE("transform.sh-*-to-typedgrid, errors") {
    EXPECT_THROWS(make("transform.sh-scalar-to-typedgrid"));  // grid is required
    EXPECT_THROWS(make("transform.sh-vod-to-uv-typedgrid"));
    EXPECT_THROWS(perform("{type: transform.sh-scalar-to-typedgrid, grid: unknown}", sh(coefficients())));
    EXPECT_THROWS(perform("{type: transform.sh-scalar-to-typedgrid, grid: O16}", field("{grid: O16}")));
}


// points file, in the test data directory (the working directory), for the action properties only
const std::string GRIDDEF = "weather-params.pts";


// few points, as transforming to unstructured grids is slow
CASE("transform.sh-scalar-to-griddef") {
    eckit::TmpFile file;
    std::ofstream(file.asString()) << "89.5 0\n60 45\n30 90\n0 180\n-45 270\n-89.5 359.5\n";
    check_scalar("{type: transform.sh-scalar-to-griddef, griddef: " + file.asString() + "}", 6);
    check_scalar("{type: transform.sh-scalar-to-griddef, griddef: " + file.asString() + "}", 6, 2);
}


CASE("transform.sh-vod-to-uv-griddef") {
    eckit::TmpFile file;
    std::ofstream(file.asString()) << "89.5 0\n60 45\n30 90\n0 180\n-45 270\n-89.5 359.5\n";
    check_vod_to_uv("{type: transform.sh-vod-to-uv-griddef, griddef: " + file.asString() + "}", 6);
    check_vod_to_uv("{type: transform.sh-vod-to-uv-griddef, griddef: " + file.asString() + "}", 6, 2);
}


CASE("transform.sh-*-to-griddef, properties") {
    auto a = make_checked("{type: transform.sh-scalar-to-griddef, griddef: " + GRIDDEF + "}");
    auto b = make_checked("{type: transform.sh-vod-to-uv-griddef, griddef: " + GRIDDEF + "}");

    EXPECT_EQUAL(std::string(a->name()), "ShToGridDef");
    EXPECT(contains(str(*a), "ShToGridDef[type=local") && contains(str(*a), ",griddef=.../" + GRIDDEF + "]"));
    EXPECT(contains(str(*b), "invtrans=<vod2uv>"));
    EXPECT(contains(a->json_str(false), R"("griddef":")" + GRIDDEF + R"(")"));

    std::string name;

    EXPECT(!a->getGriddedTargetName(name));

    EXPECT(*a != *b);
    EXPECT(*a != *make("{type: transform.sh-scalar-to-griddef, griddef: ICON.griddef}"));
    EXPECT(*a != *make("{type: transform.sh-scalar-to-griddef, atlas_trans_flt: true, griddef: " + GRIDDEF + "}"));
    EXPECT(*a != *make("{type: interpolate.grid2griddef, griddef: " + GRIDDEF + "}"));
}


CASE("transform.sh-*-to-griddef, errors") {
    EXPECT_THROWS(make("transform.sh-scalar-to-griddef"));  // griddef is required
    EXPECT_THROWS(make("transform.sh-vod-to-uv-griddef"));
    EXPECT_THROWS(perform("{type: transform.sh-scalar-to-griddef, griddef: does-not-exist.pts}", sh(coefficients())));
    EXPECT_THROWS(perform("{type: transform.sh-scalar-to-griddef, griddef: " + GRIDDEF + "}", field("{grid: O16}")));
}


const std::string POINTS = "latitudes: [89.5, 60, 30, 0, -45, -89.5], longitudes: [0, 45, 90, 180, 270, 359.5]";


// the action spec has the points by their (unstructured grid) uid, so it can't rebuild the action (by design)
CASE("transform.sh-scalar-to-points") {
    auto a = make("{type: transform.sh-scalar-to-points, " + POINTS + "}");
    check_scalar(*a, 6);
    check_scalar(*a, 6, 2);
}


CASE("transform.sh-vod-to-uv-points") {
    auto a = make("{type: transform.sh-vod-to-uv-points, " + POINTS + "}");
    check_vod_to_uv(*a, 6);
    check_vod_to_uv(*a, 6, 2);
}


CASE("transform.sh-*-to-points, properties") {
    auto a = make("{type: transform.sh-scalar-to-points, " + POINTS + "}");
    auto b = make("{type: transform.sh-vod-to-uv-points, " + POINTS + "}");

    EXPECT_EQUAL(std::string(a->name()), "ShToPoints");
    EXPECT(contains(str(*a), "ShToPoints[type=local") && contains(str(*a), ",points=6]"));
    EXPECT(contains(str(*b), "invtrans=<vod2uv>"));
    EXPECT(contains(a->json_str(false), R"("uid":")"));
    EXPECT(a->json_str(false) !=
           make("{type: transform.sh-scalar-to-points, latitudes: [0], longitudes: [0]}")->json_str(false));

    std::string name;

    EXPECT(!a->getGriddedTargetName(name));

    EXPECT(*a == *make("{type: transform.sh-scalar-to-points, " + POINTS + "}"));
    EXPECT(*a != *b);
    EXPECT(*a != *make("{type: transform.sh-scalar-to-points, latitudes: [89.5, 60, 30, 0, -45, -89], "
                       "longitudes: [0, 45, 90, 180, 270, 359.5]}"));
    EXPECT(*a != *make("{type: transform.sh-scalar-to-points, latitudes: [89.5, 60, 30, 0, -45, -89.5], "
                       "longitudes: [0, 45, 90, 180, 270, 359]}"));
    EXPECT(*a != *make("{type: interpolate.grid2points, " + POINTS + "}"));
}


CASE("transform.sh-*-to-points, errors") {
    EXPECT_THROWS(make("transform.sh-scalar-to-points"));  // latitudes and longitudes are required
    EXPECT_THROWS(make("{type: transform.sh-vod-to-uv-points, latitudes: [0, 1]}"));
    EXPECT_THROWS(make("{type: transform.sh-scalar-to-points, latitudes: [0, 1], longitudes: [0]}"));
    EXPECT_THROWS(make("{type: transform.sh-scalar-to-points, latitudes: [], longitudes: []}"));
    EXPECT_THROWS(perform(*make("{type: transform.sh-scalar-to-points, " + POINTS + "}"), field("{grid: O16}")));
}


}  // namespace mir::action::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
