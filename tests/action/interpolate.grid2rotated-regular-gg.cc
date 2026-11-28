// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <sstream>

#include "interpolate.h"


namespace mir::action::tests {


const std::string ROTATED_REGULAR_GG = "{type: interpolate.grid2rotated-regular-gg, regular: 8, rotation: [-40, 22]";


CASE("interpolate.grid2rotated-regular-gg") {
    auto a   = make_checked(ROTATED_REGULAR_GG + "}");
    auto in  = field("{grid: O16}", LATITUDE, 2);
    auto out = perform(*a, in);

    EXPECT_EQUAL(out.dimensions(), 2);
    EXPECT_EQUAL(out.values(0).size(), 512);
    EXPECT_EQUAL(out.representation()->numberOfPoints(), 512);
    EXPECT(out.representation()->uniqueName().find("F8-") == 0);
    EXPECT(out.representation()->uniqueName().find("-rot:-40:22:0") != std::string::npos);
    EXPECT(!out.hasMissing());

    // the same grid, unrotated, has the same points in the rotated frame
    auto unrotated = perform("{type: interpolate.grid2regular-gg, regular: 8}", in);

    EXPECT(out.representation()->uniqueName() != unrotated.representation()->uniqueName());

    const auto& outPoints       = out.representation()->canonicalGridUnrotated();
    const auto& unrotatedPoints = unrotated.representation()->canonicalGridUnrotated();
    EXPECT_EQUAL(outPoints.size(), unrotatedPoints.size());

    for (auto it = outPoints.cbegin(), jt = unrotatedPoints.cbegin(); it; ++it, ++jt) {
        const auto p = std::get<PointLonLat>(*it);
        const auto q = std::get<PointLonLat>(*jt);
        EXPECT(same(p.lat(), q.lat()));
        EXPECT(same(p.lon(), q.lon()));
    }
}


CASE("interpolate.grid2rotated-regular-gg, values") {
    check_values(*make_checked(ROTATED_REGULAR_GG + "}"));
    check_values(*make_checked(ROTATED_REGULAR_GG + ", interpolation: nn}"), "{grid: O16}", 3.5);
}


CASE("interpolate.grid2rotated-regular-gg, cropping") {
    auto a = make_mutable(ROTATED_REGULAR_GG + "}");

    EXPECT(!a->canCrop());
    EXPECT(a->outputBoundingBox() == util::BoundingBox(90, 0, -90, 360));

    // cropping in the rotated frame (centred on the rotated equator/Greenwich), bounding box (unrotated) containing it
    EXPECT(a->mergeWithNext(*make("{type: crop.area, area: [20, -20, -20, 20]}")));
    EXPECT(a->canCrop());
    EXPECT(!(a->outputBoundingBox() == util::BoundingBox(90, 0, -90, 360)));
    EXPECT(a->outputBoundingBox().contains(50., 22.));

    auto out = perform(*a, field("{grid: O16}", LATITUDE));

    EXPECT(0 < out.values(0).size() && out.values(0).size() < 512);
    EXPECT(contained(out, a->outputBoundingBox()));
}


CASE("interpolate.grid2rotated-regular-gg, properties") {
    auto a = make(ROTATED_REGULAR_GG + "}");

    EXPECT(a->isRegridAction());
    EXPECT(!a->canCrop());

    std::string name;

    EXPECT(!a->getGriddedTargetName(name));

    EXPECT_EQUAL(a->type(), "interpolate.grid2rotated-regular-gg");
    EXPECT_EQUAL(std::string(a->name()), "Gridded2RotatedRegularGG");
    EXPECT(a->json_str().find(R"("rotation":[-40,22],"regular":8)") != std::string::npos);

    std::ostringstream print;
    print << *a;

    EXPECT(print.str().find("Gridded2RotatedRegularGG[N=8,rotation=") == 0);
    EXPECT(print.str().find("interpolation=linear") != std::string::npos);
}


CASE("interpolate.grid2rotated-regular-gg, comparison") {
    auto a = make(ROTATED_REGULAR_GG + "}");

    EXPECT(*a == *make(ROTATED_REGULAR_GG + ", interpolation: linear}"));
    EXPECT(*a == *make("{type: interpolate.grid2rotated-regular-gg, regular: 8, rotation: -40/22}"));

    EXPECT(*a != *make("{type: interpolate.grid2rotated-regular-gg, regular: 8, rotation: [-40, 23]}"));
    EXPECT(*a != *make("{type: interpolate.grid2rotated-regular-gg, regular: 16, rotation: [-40, 22]}"));
    EXPECT(*a != *make(ROTATED_REGULAR_GG + ", interpolation: nn}"));
    EXPECT(*a != *make("{type: interpolate.grid2regular-gg, regular: 8}"));
    EXPECT(*a != *make("{type: interpolate.grid2rotated-octahedral-gg, octahedral: 8, rotation: [-40, 22]}"));
}


CASE("interpolate.grid2rotated-regular-gg, errors") {
    EXPECT_THROWS(make("{type: interpolate.grid2rotated-regular-gg, regular: 8}"));  // rotation is required
    EXPECT_THROWS(make("{type: interpolate.grid2rotated-regular-gg, rotation: [-40, 22]}"));
    EXPECT_THROWS(make("{type: interpolate.grid2rotated-regular-gg, regular: 8, rotation: [-40, 22, 0]}"));
    EXPECT_THROWS(make("{type: interpolate.grid2rotated-regular-gg, regular: 0, rotation: [-40, 22]}"));
}


}  // namespace mir::action::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
