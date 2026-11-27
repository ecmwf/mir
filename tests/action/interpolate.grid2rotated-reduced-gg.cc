// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <sstream>

#include "interpolate.h"


namespace mir::action::tests {


const std::string ROTATED_REDUCED_GG = "{type: interpolate.grid2rotated-reduced-gg, reduced: 32, rotation: [-40, 22]";


CASE("interpolate.grid2rotated-reduced-gg") {
    auto a   = make_checked(ROTATED_REDUCED_GG + "}");
    auto in  = field("{grid: O16}", LATITUDE, 2);
    auto out = perform(*a, in);

    EXPECT_EQUAL(out.dimensions(), 2);
    EXPECT_EQUAL(out.values(0).size(), 6114);
    EXPECT_EQUAL(out.representation()->numberOfPoints(), 6114);
    EXPECT(out.representation()->uniqueName().find("N32-") == 0);
    EXPECT(out.representation()->uniqueName().find("-rot:-40:22:0") != std::string::npos);
    EXPECT(!out.hasMissing());

    // the same grid, unrotated, has the same points in the rotated frame
    auto unrotated = perform("{type: interpolate.grid2reduced-gg, reduced: 32}", in);

    EXPECT(out.representation()->uniqueName() != unrotated.representation()->uniqueName());

    const auto& outPoints       = out.representation()->canonicalGridUnrotated();
    const auto& unrotatedPoints = unrotated.representation()->canonicalGridUnrotated();
    EXPECT_EQUAL(outPoints.size(), unrotatedPoints.size());

    for (auto it = outPoints.cbegin(), jt = unrotatedPoints.cbegin(); it; ++it, ++jt) {
        const auto p = std::get<PointLonLat>(*it);
        const PointLatLon q{std::get<PointLonLat>(*jt)};
        EXPECT(same(p.lat(), q.lat().value()));
        EXPECT(same(p.lon(), q.lon().value()));
    }
}


CASE("interpolate.grid2rotated-reduced-gg, values") {
    check_values(*make_checked(ROTATED_REDUCED_GG + "}"));
    check_values(*make_checked(ROTATED_REDUCED_GG + ", interpolation: nn}"), "{grid: O16}", 3.5);
}


CASE("interpolate.grid2rotated-reduced-gg, cropping") {
    auto a = make_mutable(ROTATED_REDUCED_GG + "}");

    EXPECT(!a->canCrop());
    EXPECT(a->outputBoundingBox() == util::BoundingBox(90, 0, -90, 360));

    // cropping in the rotated frame (centred on the rotated equator/Greenwich), bounding box (unrotated) containing it
    EXPECT(a->mergeWithNext(*make("{type: crop.area, area: [20, -20, -20, 20]}")));
    EXPECT(a->canCrop());
    EXPECT(!(a->outputBoundingBox() == util::BoundingBox(90, 0, -90, 360)));
    EXPECT(a->outputBoundingBox().contains(PointLatLon(50, 22)));

    auto out = perform(*a, field("{grid: O16}", LATITUDE));

    EXPECT(0 < out.values(0).size() && out.values(0).size() < 6114);
    EXPECT(contained(out, a->outputBoundingBox()));
}


CASE("interpolate.grid2rotated-reduced-gg, properties") {
    auto a = make(ROTATED_REDUCED_GG + "}");

    EXPECT(a->isRegridAction());
    EXPECT(!a->canCrop());

    std::string name;

    EXPECT(!a->getGriddedTargetName(name));

    EXPECT_EQUAL(a->type(), "interpolate.grid2rotated-reduced-gg");
    EXPECT_EQUAL(std::string(a->name()), "Gridded2RotatedReducedGG");
    EXPECT(a->json_str().find(R"("rotation":[-40,22],"reduced":32)") != std::string::npos);

    std::ostringstream print;
    print << *a;

    EXPECT(print.str().find("Gridded2RotatedReducedGG[N=32,rotation=") == 0);
    EXPECT(print.str().find("interpolation=linear") != std::string::npos);
}


CASE("interpolate.grid2rotated-reduced-gg, comparison") {
    auto a = make(ROTATED_REDUCED_GG + "}");

    EXPECT(*a == *make(ROTATED_REDUCED_GG + ", interpolation: linear}"));
    EXPECT(*a == *make("{type: interpolate.grid2rotated-reduced-gg, reduced: 32, rotation: -40/22}"));

    EXPECT(*a != *make("{type: interpolate.grid2rotated-reduced-gg, reduced: 32, rotation: [-40, 23]}"));
    EXPECT(*a != *make("{type: interpolate.grid2rotated-reduced-gg, reduced: 48, rotation: [-40, 22]}"));
    EXPECT(*a != *make(ROTATED_REDUCED_GG + ", interpolation: nn}"));
    EXPECT(*a != *make("{type: interpolate.grid2reduced-gg, reduced: 32}"));
    EXPECT(*a != *make("{type: interpolate.grid2rotated-octahedral-gg, octahedral: 32, rotation: [-40, 22]}"));
}


CASE("interpolate.grid2rotated-reduced-gg, errors") {
    EXPECT_THROWS(make("{type: interpolate.grid2rotated-reduced-gg, reduced: 32}"));  // rotation is required
    EXPECT_THROWS(make("{type: interpolate.grid2rotated-reduced-gg, rotation: [-40, 22]}"));
    EXPECT_THROWS(make("{type: interpolate.grid2rotated-reduced-gg, reduced: 32, rotation: [-40, 22, 0]}"));
    EXPECT_THROWS(make("{type: interpolate.grid2rotated-reduced-gg, reduced: 0, rotation: [-40, 22]}"));

    // N8 has no (classic) number of points per latitude
    EXPECT_THROWS(
        perform("{type: interpolate.grid2rotated-reduced-gg, reduced: 8, rotation: [-40, 22]}", field("{grid: O16}")));
}


}  // namespace mir::action::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
