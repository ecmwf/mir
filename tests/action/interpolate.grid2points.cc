// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <sstream>
#include <string>
#include <vector>

#include "interpolate.h"


namespace mir::action::tests {


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
    const auto& points = out.representation()->canonicalGridUnrotated();
    for (auto it = points.cbegin(); it; ++it) {
        const auto i = it.index();

        EXPECT(same(PointLatLon{std::get<PointLonLat>(*it)}.lat().value(), lat.at(i)));
        EXPECT(same(PointLatLon{std::get<PointLonLat>(*it)}.lon().value(), lon.at(i)));
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
    EXPECT(*a != *make("{type: interpolate.grid2gridspec, grid: {type: unstructured_ll, latitudes: [50, 60], "
                       "longitudes: [0, 0]}}"));
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
