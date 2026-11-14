// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "transform.h"


namespace mir::tests::actions::transform {


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


}  // namespace mir::tests::actions::transform


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
