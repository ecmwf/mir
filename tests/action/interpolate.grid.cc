// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <sstream>
#include <string>
#include <vector>

#include "interpolate.h"


namespace mir::action::tests {


CASE("interpolate.grid2gridspec") {
    struct test_t {
        std::string grid;
        size_t numberOfPoints;
        std::string uniqueName;  // prefix
    };

    const std::vector<test_t> tests{
        test_t{"{grid: O8}", 544, "R8-"},
        test_t{"O8", 544, "R8-"},
        test_t{"{grid: F8}", 512, "F8-"},
        test_t{"{grid: [10, 10]}", 19 * 36, "LL-10x10-90:0:-90:350"},
        test_t{"{grid: [10, 10], area: [60, -10, 30, 40]}", 4 * 6, "LL-10x10-60:-10:30:40"},
        test_t{"{grid: [10, 10], rotation: [-40, 22]}", 19 * 36, "LL-10x10-90:0:-90:350-rot:-40:22:0"},
        test_t{"{grid: H4}", 192, "H4"},
        test_t{"{grid: H4, order: nested}", 192, "HN4"},
    };
    for (const auto& test : tests) {
        SECTION(test.grid) {
            auto a   = make_checked("{type: interpolate.grid2gridspec, grid: " + test.grid + "}");
            auto out = perform(*a, field("{grid: O16}", LATITUDE, 2));

            EXPECT_EQUAL(out.dimensions(), 2);
            EXPECT_EQUAL(out.values(0).size(), test.numberOfPoints);
            EXPECT_EQUAL(out.representation()->numberOfPoints(), test.numberOfPoints);
            EXPECT(out.representation()->uniqueName().find(test.uniqueName) == 0);

            EXPECT(!out.hasMissing());
            EXPECT(max_error(out, LATITUDE, 80.) < 0.2);
            EXPECT(contained(out, a->outputBoundingBox()));
        }
    }
}


CASE("interpolate.grid2gridspec, values") {
    const std::vector<std::string> grids{"{grid: O8}", "{grid: [10, 10], area: [60, -10, 30, 40]}", "{grid: H4}"};
    for (const std::string& grid : grids) {
        SECTION(grid) {
            check_values(*make_checked("{type: interpolate.grid2gridspec, grid: " + grid + "}"));
        }
    }
}


CASE("interpolate.grid2gridspec, HEALPix methods") {
    struct test_t {
        std::string interpolation;
        double tolerance;
    };

    // H4 latitude spacing is about 12 degrees
    const std::vector<test_t> tests{
        test_t{"healpix-resample", 1.},
        test_t{"healpix-statistics, interpolation-statistics: mean", 1.},
        test_t{"healpix-statistics", 7.},
        test_t{"healpix-statistics, interpolation-statistics: minimum", 7.},
    };
    for (const auto& test : tests) {
        SECTION(test.interpolation) {
            auto a =
                make("{type: interpolate.grid2gridspec, grid: {grid: H4}, interpolation: " + test.interpolation + "}");
            EXPECT_EQUAL(make(a->json_str(false))->json_str(false), a->json_str(false));

            auto constant = perform(*a, field("{grid: H8}", CONSTANT));

            EXPECT_EQUAL(constant.values(0).size(), 192);
            EXPECT(max_error(constant, CONSTANT) < 1e-9);

            auto linear = perform(*a, field("{grid: H8}", LATITUDE));

            EXPECT(max_error(linear, LATITUDE) < test.tolerance);
        }
    }
}


CASE("interpolate.grid2gridspec, cropping") {
    auto a = make_mutable("{type: interpolate.grid2gridspec, grid: {grid: O8}}");

    EXPECT(!a->canCrop());

    EXPECT(a->mergeWithNext(*make("{type: crop.area, area: [60, -10, 30, 40]}")));
    EXPECT(a->canCrop());
    EXPECT(a->outputBoundingBox() == util::BoundingBox(60, -10, 30, 40));

    auto out = perform(*a, field("{grid: O16}", LATITUDE));

    EXPECT(0 < out.values(0).size() && out.values(0).size() < 544);
    EXPECT(contained(out, a->outputBoundingBox()));
    EXPECT(max_error(out, LATITUDE) < 0.2);
}


CASE("interpolate.grid2gridspec, properties") {
    auto a = make_checked("{type: interpolate.grid2gridspec, grid: {grid: [10, 10], area: [60, -10, 30, 40]}}");

    EXPECT(a->isRegridAction());
    EXPECT(!a->canCrop());

    std::string name;

    EXPECT(!a->getGriddedTargetName(name));

    EXPECT_EQUAL(a->type(), "interpolate.grid2gridspec");
    EXPECT_EQUAL(std::string(a->name()), "Gridded2GridSpec");

    // the geographic domain
    EXPECT(a->outputBoundingBox() == util::BoundingBox(60, -10, 30, 40));
    EXPECT(make("{type: interpolate.grid2gridspec, grid: {grid: [10, 10]}}")->outputBoundingBox() ==
           util::BoundingBox(90, 0, -90, 360));

    EXPECT(a->json_str().find(R"("grid":{)") != std::string::npos);

    std::ostringstream print;
    print << *a;

    EXPECT(print.str().find("Gridded2GridSpec[gridspec=") == 0);
    EXPECT(print.str().find("interpolation=linear") != std::string::npos);
}


CASE("interpolate.grid2gridspec, comparison") {
    auto a = make("{type: interpolate.grid2gridspec, grid: {grid: O8}}");

    EXPECT(*a == *make("{type: interpolate.grid2gridspec, grid: O8}"));
    EXPECT(*a == *make("{type: interpolate.grid2gridspec, grid: {grid: O8}, interpolation: linear}"));

    EXPECT(*a != *make("{type: interpolate.grid2gridspec, grid: {grid: O16}}"));
    EXPECT(*a != *make("{type: interpolate.grid2gridspec, grid: {grid: O8}, interpolation: nn}"));
    EXPECT(*a != *make("{type: interpolate.grid2namedgrid, grid: O8}"));
}


CASE("interpolate.grid2gridspec, errors") {
    EXPECT_THROWS(make("interpolate.grid2gridspec"));                           // grid is required
    EXPECT_THROWS(make("{type: interpolate.grid2gridspec, grid: {grid: ?}}"));  // unknown grid
}


CASE("interpolate.grid2namedgrid") {
    struct test_t {
        std::string grid;
        std::string name;  // canonical
        size_t numberOfPoints;
    };

    const std::vector<test_t> tests{
        test_t{"O8", "O8", 544},    test_t{"o8", "O8", 544},           test_t{"F8", "F8", 512},
        test_t{"N32", "N32", 6114}, test_t{"10/10", "10/10", 19 * 36},
    };
    for (const auto& test : tests) {
        SECTION(test.grid) {
            auto a = make_checked("{type: interpolate.grid2namedgrid, grid: " + test.grid + "}");

            std::string name;

            EXPECT(a->getGriddedTargetName(name));
            EXPECT_EQUAL(name, test.name);

            auto out = perform(*a, field("{grid: O16}", LATITUDE, 2));

            EXPECT_EQUAL(out.dimensions(), 2);
            EXPECT_EQUAL(out.values(0).size(), test.numberOfPoints);
            EXPECT_EQUAL(out.representation()->numberOfPoints(), test.numberOfPoints);
            EXPECT(!out.hasMissing());
            EXPECT(contained(out, a->outputBoundingBox()));
        }
    }
}


CASE("interpolate.grid2namedgrid, values") {
    const std::vector<std::string> grids{"O8", "F8", "N32"};
    for (const std::string& grid : grids) {
        SECTION(grid) {
            check_values(*make_checked("{type: interpolate.grid2namedgrid, grid: " + grid + "}"));
        }
    }
}


CASE("interpolate.grid2namedgrid, cropping") {
    auto a = make_mutable("{type: interpolate.grid2namedgrid, grid: O8}");

    EXPECT(!a->canCrop());

    EXPECT(a->mergeWithNext(*make("{type: crop.area, area: [60, -10, 30, 40]}")));
    EXPECT(a->canCrop());
    EXPECT(a->outputBoundingBox() == util::BoundingBox(60, -10, 30, 40));

    auto out = perform(*a, field("{grid: O16}", LATITUDE));

    EXPECT(0 < out.values(0).size() && out.values(0).size() < 544);
    EXPECT(contained(out, a->outputBoundingBox()));
}


CASE("interpolate.grid2namedgrid, properties") {
    auto a = make("{type: interpolate.grid2namedgrid, grid: o8}");

    EXPECT(a->isRegridAction());
    EXPECT(!a->canCrop());
    EXPECT_EQUAL(a->type(), "interpolate.grid2namedgrid");
    EXPECT_EQUAL(std::string(a->name()), "Gridded2NamedGrid");
    EXPECT(a->outputBoundingBox() == util::BoundingBox(90, 0, -90, 360));
    EXPECT(a->json_str().find(R"("grid":"O8")") != std::string::npos);

    std::ostringstream print;
    print << *a;

    EXPECT(print.str().find("Gridded2NamedGrid[grid=O8,interpolation=linear") == 0);
}


CASE("interpolate.grid2namedgrid, comparison") {
    auto a = make("{type: interpolate.grid2namedgrid, grid: O8}");

    EXPECT(*a == *make("{type: interpolate.grid2namedgrid, grid: o8}"));
    EXPECT(*a == *make("{type: interpolate.grid2namedgrid, grid: O8, interpolation: linear}"));

    EXPECT(*a != *make("{type: interpolate.grid2namedgrid, grid: O16}"));
    EXPECT(*a != *make("{type: interpolate.grid2namedgrid, grid: N8}"));
    EXPECT(*a != *make("{type: interpolate.grid2namedgrid, grid: O8, interpolation: nn}"));
    EXPECT(*a != *make("{type: interpolate.grid2octahedral-gg, octahedral: 8}"));
    EXPECT(*a != *make("{type: interpolate.grid2rotated-namedgrid, grid: O8, rotation: [-40, 22]}"));
}


CASE("interpolate.grid2namedgrid, errors") {
    EXPECT_THROWS(make("interpolate.grid2namedgrid"));                         // grid is required
    EXPECT_THROWS(make("{type: interpolate.grid2namedgrid, grid: unknown}"));  // unknown grid

    // N8 has no (classic) number of points per latitude
    EXPECT_THROWS(perform("{type: interpolate.grid2namedgrid, grid: N8}", field("{grid: O16}")));
}


const std::string ROTATED_NAMEDGRID = "{type: interpolate.grid2rotated-namedgrid, grid: O8, rotation: [-40, 22]";


CASE("interpolate.grid2rotated-namedgrid") {
    auto a   = make_checked(ROTATED_NAMEDGRID + "}");
    auto in  = field("{grid: O16}", LATITUDE, 2);
    auto out = perform(*a, in);

    EXPECT_EQUAL(out.dimensions(), 2);
    EXPECT_EQUAL(out.values(0).size(), 544);
    EXPECT_EQUAL(out.representation()->numberOfPoints(), 544);
    EXPECT(out.representation()->uniqueName().find("O8-") == 0);
    EXPECT(out.representation()->uniqueName().find("-rot:-40:22:0") != std::string::npos);
    EXPECT(!out.hasMissing());

    // the same grid, unrotated, has the same points in the rotated frame
    auto unrotated = perform("{type: interpolate.grid2namedgrid, grid: O8}", in);

    EXPECT(out.representation()->uniqueName() != unrotated.representation()->uniqueName());

    std::unique_ptr<repres::Iterator> it(out.representation()->iterator());
    std::unique_ptr<repres::Iterator> jt(unrotated.representation()->iterator());

    while (it->next()) {
        EXPECT(jt->next());
        EXPECT(same(it->pointUnrotated().lat().value(), jt->pointUnrotated().lat().value()));
        EXPECT(same(it->pointUnrotated().lon().value(), jt->pointUnrotated().lon().value()));
    }

    EXPECT(!jt->next());

    // the same as by type
    auto typed = perform("{type: interpolate.grid2rotated-octahedral-gg, octahedral: 8, rotation: [-40, 22]}", in);

    EXPECT_EQUAL(out.representation()->uniqueName(), typed.representation()->uniqueName());
    EXPECT(out.values(0) == typed.values(0));

    const std::vector<std::string> grids{"o8", "F8", "N32"};
    for (const std::string& grid : grids) {
        EXPECT(perform("{type: interpolate.grid2rotated-namedgrid, grid: " + grid + ", rotation: [-40, 22]}", in)
                   .representation()
                   ->uniqueName()
                   .find("-rot:-40:22:0") != std::string::npos);
    }
}


CASE("interpolate.grid2rotated-namedgrid, values") {
    check_values(*make_checked(ROTATED_NAMEDGRID + "}"));
    check_values(*make_checked(ROTATED_NAMEDGRID + ", interpolation: nn}"), "{grid: O16}", 3.5);
}


CASE("interpolate.grid2rotated-namedgrid, cropping") {
    auto a = make_mutable(ROTATED_NAMEDGRID + "}");

    EXPECT(!a->canCrop());
    EXPECT(a->outputBoundingBox() == util::BoundingBox(90, 0, -90, 360));

    // cropping in the rotated frame (centred on the rotated equator/Greenwich), bounding box (unrotated) containing it
    EXPECT(a->mergeWithNext(*make("{type: crop.area, area: [20, -20, -20, 20]}")));
    EXPECT(a->canCrop());
    EXPECT(!(a->outputBoundingBox() == util::BoundingBox(90, 0, -90, 360)));
    EXPECT(a->outputBoundingBox().contains(PointLatLon(50, 22)));

    auto out = perform(*a, field("{grid: O16}", LATITUDE));

    EXPECT(0 < out.values(0).size() && out.values(0).size() < 544);
    EXPECT(contained(out, a->outputBoundingBox()));
}


CASE("interpolate.grid2rotated-namedgrid, properties") {
    auto a = make(ROTATED_NAMEDGRID + "}");

    EXPECT(a->isRegridAction());
    EXPECT(!a->canCrop());

    std::string name;

    EXPECT(!a->getGriddedTargetName(name));

    EXPECT_EQUAL(a->type(), "interpolate.grid2rotated-namedgrid");
    EXPECT_EQUAL(std::string(a->name()), "Gridded2RotatedNamedGrid");
    EXPECT(a->json_str().find(R"("rotation":[-40,22],"grid":"O8")") != std::string::npos);

    std::ostringstream print;
    print << *a;

    EXPECT(print.str().find("Gridded2RotatedNamedGrid[grid=O8,rotation=") == 0);
    EXPECT(print.str().find("interpolation=linear") != std::string::npos);
}


CASE("interpolate.grid2rotated-namedgrid, comparison") {
    auto a = make(ROTATED_NAMEDGRID + "}");

    EXPECT(*a == *make(ROTATED_NAMEDGRID + ", interpolation: linear}"));
    EXPECT(*a == *make("{type: interpolate.grid2rotated-namedgrid, grid: O8, rotation: -40/22}"));

    EXPECT(*a != *make("{type: interpolate.grid2rotated-namedgrid, grid: O8, rotation: [-40, 23]}"));
    EXPECT(*a != *make("{type: interpolate.grid2rotated-namedgrid, grid: O16, rotation: [-40, 22]}"));
    EXPECT(*a != *make(ROTATED_NAMEDGRID + ", interpolation: nn}"));
    EXPECT(*a != *make("{type: interpolate.grid2namedgrid, grid: O8}"));
    EXPECT(*a == *make("{type: interpolate.grid2rotated-namedgrid, grid: o8, rotation: [-40, 22]}"));
    EXPECT(*a != *make("{type: interpolate.grid2rotated-octahedral-gg, octahedral: 8, rotation: [-40, 22]}"));
}


CASE("interpolate.grid2rotated-namedgrid, errors") {
    EXPECT_THROWS(make("{type: interpolate.grid2rotated-namedgrid, grid: O8}"));  // rotation is required
    EXPECT_THROWS(make("{type: interpolate.grid2rotated-namedgrid, rotation: [-40, 22]}"));
    EXPECT_THROWS(make("{type: interpolate.grid2rotated-namedgrid, grid: O8, rotation: [-40, 22, 0]}"));
    EXPECT_THROWS(make("{type: interpolate.grid2rotated-namedgrid, grid: unknown, rotation: [-40, 22]}"));
}


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


// griddef files (in tests/data): a tour of the Azores, and a global set of points (including the poles)
const std::string TOUR   = "tour.pts";
const std::string GLOBAL = "gridType=unstructured_grid.pts";
const std::string AZORES = "[40, -32, 36, -24]";


CASE("interpolate.grid2griddef") {
    struct test_t {
        std::string griddef;
        size_t numberOfPoints;
        double latitude;  // of the first point
        double longitude;
    };

    const std::vector<test_t> tests{
        test_t{TOUR, 34, 37.742114, -25.664350},
        test_t{GLOBAL, 42, 90., 0.},
    };
    for (const auto& test : tests) {
        SECTION(test.griddef) {
            auto out = perform("{type: interpolate.grid2griddef, griddef: '" + test.griddef + "'}",
                               field("{grid: O16}", LATITUDE, 2));

            EXPECT_EQUAL(out.dimensions(), 2);
            EXPECT_EQUAL(out.values(0).size(), test.numberOfPoints);
            EXPECT_EQUAL(out.representation()->numberOfPoints(), test.numberOfPoints);
            EXPECT(out.representation()->uniqueName().find("unstructured-" + std::to_string(test.numberOfPoints)) == 0);

            std::unique_ptr<repres::Iterator> it(out.representation()->iterator());

            EXPECT(it->next());
            EXPECT(same(it->pointUnrotated().lat().value(), test.latitude));
            EXPECT(same(it->pointUnrotated().lon().value(), test.longitude));
        }
    }
}


CASE("interpolate.grid2griddef, values") {
    SECTION(TOUR) {
        check_values(*make_checked("{type: interpolate.grid2griddef, griddef: " + TOUR + "}"));
    }

    SECTION(GLOBAL) {
        // the poles are outside of the O16 latitudes
        check_values(*make_checked("{type: interpolate.grid2griddef, griddef: '" + GLOBAL + "'}"));
    }

    SECTION("nearest neighbour") {
        check_values(*make_checked("{type: interpolate.grid2griddef, griddef: " + TOUR + ", interpolation: nn}"),
                     "{grid: O16}", 3.);
    }
}


CASE("interpolate.grid2griddef, missing values") {
    auto out = perform("{type: interpolate.grid2griddef, griddef: '" + GLOBAL + "'}",
                       field("{grid: O16}", MISSING_NORTH, 1, 9999.));

    EXPECT(out.hasMissing());
    EXPECT_EQUAL(count_missing(out), 12);  // points at 60N and 90N
}


CASE("interpolate.grid2griddef, cropping") {
    auto a = make_mutable("{type: interpolate.grid2griddef, griddef: " + TOUR + "}");

    EXPECT(!a->canCrop());
    EXPECT(a->outputBoundingBox() == util::BoundingBox(90, 0, -90, 360));

    EXPECT(a->mergeWithNext(*make("{type: crop.area, area: " + AZORES + "}")));
    EXPECT(a->canCrop());
    EXPECT(a->outputBoundingBox() == util::BoundingBox(40, -32, 36, -24));

    auto out = perform(*a, field("{grid: O16}", LATITUDE));

    EXPECT_EQUAL(out.values(0).size(), 34);
    EXPECT(contained(out, a->outputBoundingBox()));
}


CASE("interpolate.grid2griddef, properties") {
    auto a = make_checked("{type: interpolate.grid2griddef, griddef: " + TOUR + "}");

    EXPECT(a->isRegridAction());
    EXPECT(!a->canCrop());

    std::string name;

    EXPECT(!a->getGriddedTargetName(name));

    EXPECT_EQUAL(a->type(), "interpolate.grid2griddef");
    EXPECT_EQUAL(std::string(a->name()), "Gridded2GridDef");
    EXPECT(a->json_str().find(R"("griddef":"tour.pts")") != std::string::npos);

    std::ostringstream print;
    print << *a;

    EXPECT(print.str().find("Gridded2GridDef[griddef=tour.pts,interpolation=linear") == 0);

    // the path is abbreviated
    std::ostringstream custom;
    make("{type: interpolate.grid2griddef, griddef: ../data/" + TOUR + "}")->custom(custom);

    EXPECT_EQUAL(custom.str(), "Gridded2GridDef[griddef=.../tour.pts]");
}


CASE("interpolate.grid2griddef, comparison") {
    auto a = make("{type: interpolate.grid2griddef, griddef: " + TOUR + "}");

    EXPECT(*a == *make("{type: interpolate.grid2griddef, griddef: " + TOUR + ", interpolation: linear}"));
    EXPECT(*a != *make("{type: interpolate.grid2griddef, griddef: '" + GLOBAL + "'}"));
    EXPECT(*a != *make("{type: interpolate.grid2griddef, griddef: " + TOUR + ", interpolation: nn}"));
    EXPECT(*a != *make("{type: interpolate.grid2namedgrid, grid: O16}"));
}


CASE("interpolate.grid2griddef, errors") {
    EXPECT_THROWS(make("interpolate.grid2griddef"));  // griddef is required

    // the griddef is read on use
    auto in = field("{grid: O16}");

    EXPECT_THROWS(perform("{type: interpolate.grid2griddef, griddef: does-not-exist.pts}", in));
    EXPECT_THROWS(perform("{type: interpolate.grid2griddef, griddef: /dev/null}", in));
}


const std::string POINTS = "latitudes: [10, 20, 30, -60, 0], longitudes: [0, 10, 20, 300, 180]";


// the spec identifies the points by their unique id, not by their coordinates (see json): actions are built by make,
// not make_checked
CASE("interpolate.grid2points") {
    const std::vector<double> lat{10, 20, 30, -60, 0};
    const std::vector<double> lon{0, 10, 20, 300, 180};

    auto a   = make("{type: interpolate.grid2points, " + POINTS + "}");
    auto out = perform(*a, field("{grid: O16}", LATITUDE, 2));

    EXPECT_EQUAL(out.dimensions(), 2);
    EXPECT_EQUAL(out.values(0).size(), 5);
    EXPECT_EQUAL(out.representation()->numberOfPoints(), 5);
    EXPECT(out.representation()->uniqueName().find("unstructured-5-") == 0);

    // the points, in order
    for (std::unique_ptr<repres::Iterator> it(out.representation()->iterator()); it->next();) {
        const auto i = it->index();

        EXPECT(same(it->pointUnrotated().lat().value(), lat.at(i)));
        EXPECT(same(it->pointUnrotated().lon().value(), lon.at(i)));
        EXPECT(std::abs(out.values(0)[i] - lat.at(i)) < 0.2);
    }
}


CASE("interpolate.grid2points, values") {
    check_values(*make("{type: interpolate.grid2points, " + POINTS + "}"));
    check_values(*make("{type: interpolate.grid2points, " + POINTS + ", interpolation: nn}"), "{grid: O16}", 3.5);

    auto out = perform(*make("{type: interpolate.grid2points, latitudes: [35, 60], longitudes: [0, 0]}"),
                       field("{grid: O16}", MISSING_NORTH, 1, 9999.));
    EXPECT(out.hasMissing());
    EXPECT(out.values(0)[0] != 9999.);
    EXPECT(out.values(0)[1] == 9999.);
}


CASE("interpolate.grid2points, cropping") {
    auto a = make_mutable("{type: interpolate.grid2points, " + POINTS + "}");

    EXPECT(!a->canCrop());

    EXPECT(a->mergeWithNext(*make("{type: crop.area, area: [40, -10, 0, 40]}")));
    EXPECT(a->canCrop());
    EXPECT(a->outputBoundingBox() == util::BoundingBox(40, -10, 0, 40));

    auto out = perform(*a, field("{grid: O16}", LATITUDE));

    EXPECT_EQUAL(out.values(0).size(), 3);
    EXPECT(contained(out, a->outputBoundingBox()));
}


CASE("interpolate.grid2points, properties") {
    auto a = make("{type: interpolate.grid2points, " + POINTS + "}");

    EXPECT(a->isRegridAction());
    EXPECT(!a->canCrop());

    std::string name;

    EXPECT(!a->getGriddedTargetName(name));

    EXPECT_EQUAL(a->type(), "interpolate.grid2points");
    EXPECT_EQUAL(std::string(a->name()), "Gridded2Points");
    EXPECT(a->outputBoundingBox() == util::BoundingBox(90, 0, -90, 360));

    // the points, by their unique id
    const auto json = a->json_str();

    EXPECT(json.find(R"("uid":")") != std::string::npos);
    EXPECT(json.find("latitudes") == std::string::npos);
    EXPECT(json == make("{type: interpolate.grid2points, " + POINTS + "}")->json_str());
    EXPECT(json != make("{type: interpolate.grid2points, latitudes: [50, 60], longitudes: [0, 0]}")->json_str());

    std::ostringstream print;
    print << *a;

    EXPECT(print.str().find("Gridded2Points[points=5,interpolation=linear") == 0);
}


CASE("interpolate.grid2points, comparison") {
    auto a = make("{type: interpolate.grid2points, latitudes: [50, 60], longitudes: [0, 0]}");

    EXPECT(*a == *make("{type: interpolate.grid2points, latitudes: [50, 60], longitudes: [0, 0]}"));
    EXPECT(*a == *make("{type: interpolate.grid2points, latitudes: 50/60, longitudes: 0/0}"));

    EXPECT(*a != *make("{type: interpolate.grid2points, latitudes: [50, 61], longitudes: [0, 0]}"));
    EXPECT(*a != *make("{type: interpolate.grid2points, latitudes: [50, 60], longitudes: [0, 1]}"));
    EXPECT(*a != *make("{type: interpolate.grid2points, latitudes: [50, 60], longitudes: [0, 0], interpolation: nn}"));
    EXPECT(*a != *make("{type: interpolate.grid2griddef, griddef: tour.pts}"));
}


CASE("interpolate.grid2points, errors") {
    EXPECT_THROWS(make("interpolate.grid2points"));
    EXPECT_THROWS(make("{type: interpolate.grid2points, latitudes: [50, 60]}"));
    EXPECT_THROWS(make("{type: interpolate.grid2points, longitudes: [0, 0]}"));
    EXPECT_THROWS(make("{type: interpolate.grid2points, latitudes: [50, 60], longitudes: [0]}"));

    // duplicate points
    EXPECT_THROWS(perform(*make("{type: interpolate.grid2points, latitudes: [50, 50], longitudes: [0, 0]}"),
                          field("{grid: O16}")));
}


}  // namespace mir::action::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
