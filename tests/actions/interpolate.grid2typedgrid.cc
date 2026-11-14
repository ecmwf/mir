// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <sstream>
#include <string>
#include <vector>

#include "interpolate.h"


namespace mir::tests::actions {


const std::string LAEA =
    "gridType=lambert_azimuthal_equal_area;grid=100000/100000;standardParallelInDegrees=52;"
    "centralLongitudeInDegrees=10;latitudeOfFirstGridPointInDegrees=66.982143;"
    "longitudeOfFirstGridPointInDegrees=-35.034024;Ni=40;Nj=38";

const std::string POLAR_STEREOGRAPHIC =
    "gridType=polar_stereographic;LaDInDegrees=60;orientationOfTheGridInDegrees=0;southPoleOnProjectionPlane=0;"
    "grid=100000/100000;latitudeOfFirstGridPointInDegrees=60;longitudeOfFirstGridPointInDegrees=-45;Ni=20;Nj=20";

const std::string LAMBERT =
    "gridType=lambert;LaDInDegrees=50;LoVInDegrees=10;Latin1InDegrees=50;Latin2InDegrees=50;grid=100000/100000;"
    "latitudeOfFirstGridPointInDegrees=40;longitudeOfFirstGridPointInDegrees=-10;Ni=20;Nj=15";


std::string spec(const std::string& grid, const std::string& options = "") {
    return "{type: interpolate.grid2typedgrid, grid: '" + grid + "'" + options + "}";
}


CASE("interpolate.grid2typedgrid") {
    struct test_t {
        std::string name;
        std::string grid;
        size_t numberOfPoints;
    };

    const std::vector<test_t> tests{
        test_t{"lambert_azimuthal_equal_area", LAEA, 40 * 38},
        test_t{"polar_stereographic", POLAR_STEREOGRAPHIC, 20 * 20},
        test_t{"lambert", LAMBERT, 20 * 15},
    };
    for (const auto& test : tests) {
        SECTION(test.name) {
            auto a   = make_checked(spec(test.grid));
            auto out = perform(*a, field("{grid: O16}", LATITUDE, 2));

            EXPECT_EQUAL(out.dimensions(), 2);
            EXPECT_EQUAL(out.values(0).size(), test.numberOfPoints);
            EXPECT_EQUAL(out.representation()->numberOfPoints(), test.numberOfPoints);
            EXPECT(out.representation()->uniqueName().find("RegularXY-") == 0);
            EXPECT(!out.hasMissing());

            // the grid corners are on the bounding box, contained only within (projection) rounding
            EXPECT(a->outputBoundingBox() == out.representation()->boundingBox());
        }
    }
}


CASE("interpolate.grid2typedgrid, values") {
    const std::vector<std::string> grids{LAEA, POLAR_STEREOGRAPHIC, LAMBERT};
    for (const auto& grid : grids) {
        SECTION(grid.substr(0, grid.find(';'))) {
            check_values(*make_checked(spec(grid)));
            check_values(*make_checked(spec(grid, ", interpolation: nn")), "{grid: O16}", 3.5);
        }
    }
}


CASE("interpolate.grid2typedgrid, properties") {
    auto a = make(spec(LAEA));

    EXPECT(a->isRegridAction());
    EXPECT(!a->canCrop());

    std::string name;

    EXPECT(!a->getGriddedTargetName(name));

    EXPECT_EQUAL(a->type(), "interpolate.grid2typedgrid");
    EXPECT_EQUAL(std::string(a->name()), "Gridded2TypedGrid");
    EXPECT(a->json_str().find(R"("grid":")" + LAEA + '"') != std::string::npos);

    std::ostringstream print;
    print << *a;

    EXPECT(print.str().find("Gridded2TypedGrid[grid=") == 0);
    EXPECT(print.str().find("interpolation=linear") != std::string::npos);
}


CASE("interpolate.grid2typedgrid, comparison") {
    auto a = make(spec(POLAR_STEREOGRAPHIC));

    EXPECT(*a == *make(spec(POLAR_STEREOGRAPHIC, ", interpolation: linear")));

    // the same keys, in a different order
    EXPECT(*a == *make(spec("gridType=polar_stereographic;Ni=20;Nj=20;LaDInDegrees=60;orientationOfTheGridInDegrees=0;"
                            "southPoleOnProjectionPlane=0;grid=100000/100000;latitudeOfFirstGridPointInDegrees=60;"
                            "longitudeOfFirstGridPointInDegrees=-45")));

    auto other = POLAR_STEREOGRAPHIC;
    other.replace(other.find("Nj=20"), 5, "Nj=21");

    EXPECT(*a != *make(spec(other)));

    EXPECT(*a != *make(spec(POLAR_STEREOGRAPHIC, ", interpolation: nn")));
    EXPECT(*a != *make(spec(LAEA)));
    EXPECT(*a != *make("{type: interpolate.grid2namedgrid, grid: O16}"));
}


CASE("interpolate.grid2typedgrid, errors") {
    EXPECT_THROWS(make("interpolate.grid2typedgrid"));           // grid is required
    EXPECT_THROWS(make(spec("gridType=unknown;Ni=20;Nj=20")));   // unknown type
    EXPECT_THROWS(make(spec(LAEA.substr(0, LAEA.rfind(';')))));  // required key missing
    EXPECT_THROWS(make(spec(LAEA + ";unknown=1")));              // unknown key
    EXPECT_THROWS(make(spec(LAEA + ";Ni")));                     // not key=value
}


}  // namespace mir::tests::actions


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
