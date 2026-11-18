// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <sstream>

#include "interpolate.h"


namespace mir::action::tests {


const std::string ROTATED_OCTAHEDRAL_GG =
    "{type: interpolate.grid2rotated-octahedral-gg, octahedral: 8, rotation: [-40, 22]";


CASE("interpolate.grid2rotated-octahedral-gg") {
    auto a   = make_checked(ROTATED_OCTAHEDRAL_GG + "}");
    auto in  = field("{grid: O16}", LATITUDE, 2);
    auto out = perform(*a, in);

    EXPECT_EQUAL(out.dimensions(), 2);
    EXPECT_EQUAL(out.values(0).size(), 544);
    EXPECT_EQUAL(out.representation()->numberOfPoints(), 544);
    EXPECT(out.representation()->uniqueName().find("O8-") == 0);
    EXPECT(out.representation()->uniqueName().find("-rot:-40:22:0") != std::string::npos);
    EXPECT(!out.hasMissing());

    // the same grid, unrotated, has the same points in the rotated frame
    auto unrotated = perform("{type: interpolate.grid2octahedral-gg, octahedral: 8}", in);

    EXPECT(out.representation()->uniqueName() != unrotated.representation()->uniqueName());

    std::unique_ptr<repres::Iterator> it(out.representation()->iterator());
    std::unique_ptr<repres::Iterator> jt(unrotated.representation()->iterator());

    while (it->next()) {
        EXPECT(jt->next());
        EXPECT(same(it->pointUnrotated().lat().value(), jt->pointUnrotated().lat().value()));
        EXPECT(same(it->pointUnrotated().lon().value(), jt->pointUnrotated().lon().value()));
    }

    EXPECT(!jt->next());
}


CASE("interpolate.grid2rotated-octahedral-gg, values") {
    check_values(*make_checked(ROTATED_OCTAHEDRAL_GG + "}"));
    check_values(*make_checked(ROTATED_OCTAHEDRAL_GG + ", interpolation: nn}"), "{grid: O16}", 3.5);
}


CASE("interpolate.grid2rotated-octahedral-gg, cropping") {
    auto a = make_mutable(ROTATED_OCTAHEDRAL_GG + "}");

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


CASE("interpolate.grid2rotated-octahedral-gg, properties") {
    auto a = make(ROTATED_OCTAHEDRAL_GG + "}");

    EXPECT(a->isRegridAction());
    EXPECT(!a->canCrop());

    std::string name;

    EXPECT(!a->getGriddedTargetName(name));

    EXPECT_EQUAL(a->type(), "interpolate.grid2rotated-octahedral-gg");
    EXPECT_EQUAL(std::string(a->name()), "Gridded2RotatedOctahedralGG");
    EXPECT(a->json_str().find(R"("rotation":[-40,22],"octahedral":8)") != std::string::npos);

    std::ostringstream print;
    print << *a;

    EXPECT(print.str().find("Gridded2RotatedOctahedralGG[N=8,rotation=") == 0);
    EXPECT(print.str().find("interpolation=linear") != std::string::npos);
}


CASE("interpolate.grid2rotated-octahedral-gg, comparison") {
    auto a = make(ROTATED_OCTAHEDRAL_GG + "}");

    EXPECT(*a == *make(ROTATED_OCTAHEDRAL_GG + ", interpolation: linear}"));
    EXPECT(*a == *make("{type: interpolate.grid2rotated-octahedral-gg, octahedral: 8, rotation: -40/22}"));

    EXPECT(*a != *make("{type: interpolate.grid2rotated-octahedral-gg, octahedral: 8, rotation: [-40, 23]}"));
    EXPECT(*a != *make("{type: interpolate.grid2rotated-octahedral-gg, octahedral: 16, rotation: [-40, 22]}"));
    EXPECT(*a != *make(ROTATED_OCTAHEDRAL_GG + ", interpolation: nn}"));
    EXPECT(*a != *make("{type: interpolate.grid2octahedral-gg, octahedral: 8}"));
    EXPECT(*a != *make("{type: interpolate.grid2rotated-regular-gg, regular: 8, rotation: [-40, 22]}"));
}


CASE("interpolate.grid2rotated-octahedral-gg, errors") {
    EXPECT_THROWS(make("{type: interpolate.grid2rotated-octahedral-gg, octahedral: 8}"));  // rotation is required
    EXPECT_THROWS(make("{type: interpolate.grid2rotated-octahedral-gg, rotation: [-40, 22]}"));
    EXPECT_THROWS(make("{type: interpolate.grid2rotated-octahedral-gg, octahedral: 8, rotation: [-40, 22, 0]}"));
    EXPECT_THROWS(make("{type: interpolate.grid2rotated-octahedral-gg, octahedral: 0, rotation: [-40, 22]}"));
}


}  // namespace mir::action::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
