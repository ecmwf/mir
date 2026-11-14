// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <fstream>

#include "eckit/filesystem/TmpFile.h"

#include "transform.h"


namespace mir::tests::actions::transform {


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


}  // namespace mir::tests::actions::transform


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
