// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <sstream>
#include <vector>

#include "interpolate.h"


namespace mir::action::tests {


CASE("interpolate.grid2regular-gg") {
    struct test_t {
        size_t N;
        size_t numberOfPoints;
    };

    const std::vector<test_t> tests{test_t{8, 512}, test_t{16, 2048}};
    for (const auto& test : tests) {
        const auto N = std::to_string(test.N);

        SECTION("regular: " + N) {
            auto a = make_checked("{type: interpolate.grid2regular-gg, regular: " + N + "}");

            std::string name;

            EXPECT(a->getGriddedTargetName(name));
            EXPECT_EQUAL(name, "F" + N);

            auto in  = field("{grid: O16}", LATITUDE, 2);
            auto out = perform(*a, in);

            EXPECT_EQUAL(out.dimensions(), 2);
            EXPECT_EQUAL(out.values(0).size(), test.numberOfPoints);
            EXPECT_EQUAL(out.representation()->numberOfPoints(), test.numberOfPoints);
            EXPECT(!out.hasMissing());

            // the same as the named grid
            auto named = perform("{type: interpolate.grid2namedgrid, grid: F" + N + "}", in);

            EXPECT(out.representation()->sameAs(*named.representation()));
            EXPECT(out.values(0) == named.values(0));
        }
    }
}


CASE("interpolate.grid2regular-gg, values") {
    check_values(*make_checked("{type: interpolate.grid2regular-gg, regular: 8}"));
    check_values(*make_checked("{type: interpolate.grid2regular-gg, regular: 8, interpolation: nn}"), "{grid: O16}",
                 3.5);
}


CASE("interpolate.grid2regular-gg, cropping") {
    auto a = make_mutable("{type: interpolate.grid2regular-gg, regular: 8}");

    EXPECT(!a->canCrop());

    EXPECT(a->mergeWithNext(*make("{type: crop.area, area: [60, -10, 30, 40]}")));
    EXPECT(a->canCrop());
    EXPECT(a->outputBoundingBox() == util::BoundingBox(60, -10, 30, 40));

    auto out = perform(*a, field("{grid: O16}", LATITUDE));

    EXPECT(0 < out.values(0).size() && out.values(0).size() < 512);
    EXPECT(contained(out, a->outputBoundingBox()));
}


CASE("interpolate.grid2regular-gg, properties") {
    auto a = make("{type: interpolate.grid2regular-gg, regular: 8}");

    EXPECT(a->isRegridAction());
    EXPECT(!a->canCrop());
    EXPECT_EQUAL(a->type(), "interpolate.grid2regular-gg");
    EXPECT_EQUAL(std::string(a->name()), "Gridded2RegularGG");
    EXPECT(a->outputBoundingBox() == util::BoundingBox(90, 0, -90, 360));
    EXPECT(a->json_str().find(R"("regular":8)") != std::string::npos);

    std::ostringstream print;
    print << *a;

    EXPECT(print.str().find("Gridded2RegularGG[N=8,interpolation=linear") == 0);
}


CASE("interpolate.grid2regular-gg, comparison") {
    auto a = make("{type: interpolate.grid2regular-gg, regular: 8}");

    EXPECT(*a == *make("{type: interpolate.grid2regular-gg, regular: 8, interpolation: linear}"));

    EXPECT(*a != *make("{type: interpolate.grid2regular-gg, regular: 16}"));
    EXPECT(*a != *make("{type: interpolate.grid2regular-gg, regular: 8, interpolation: nn}"));
    EXPECT(*a != *make("{type: interpolate.grid2namedgrid, grid: F8}"));
    EXPECT(*a != *make("{type: interpolate.grid2rotated-regular-gg, regular: 8, rotation: [-40, 22]}"));
    EXPECT(*a != *make("{type: interpolate.grid2octahedral-gg, octahedral: 8}"));
}


CASE("interpolate.grid2regular-gg, errors") {
    EXPECT_THROWS(make("interpolate.grid2regular-gg"));  // regular is required
    EXPECT_THROWS(make("{type: interpolate.grid2regular-gg, regular: 0}"));
}


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

    std::unique_ptr<repres::Iterator> it(out.representation()->iterator());
    std::unique_ptr<repres::Iterator> jt(unrotated.representation()->iterator());

    while (it->next()) {
        EXPECT(jt->next());
        EXPECT(same(it->pointUnrotated().lat().value(), jt->pointUnrotated().lat().value()));
        EXPECT(same(it->pointUnrotated().lon().value(), jt->pointUnrotated().lon().value()));
    }

    EXPECT(!jt->next());
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
    EXPECT(a->outputBoundingBox().contains(PointLatLon(50, 22)));

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


CASE("interpolate.grid2octahedral-gg") {
    struct test_t {
        size_t N;
        size_t numberOfPoints;
    };

    const std::vector<test_t> tests{test_t{8, 544}, test_t{16, 1600}};
    for (const auto& test : tests) {
        const auto N = std::to_string(test.N);

        SECTION("octahedral: " + N) {
            auto a = make_checked("{type: interpolate.grid2octahedral-gg, octahedral: " + N + "}");

            std::string name;

            EXPECT(a->getGriddedTargetName(name));
            EXPECT_EQUAL(name, "O" + N);

            auto in  = field("{grid: O16}", LATITUDE, 2);
            auto out = perform(*a, in);

            EXPECT_EQUAL(out.dimensions(), 2);
            EXPECT_EQUAL(out.values(0).size(), test.numberOfPoints);
            EXPECT_EQUAL(out.representation()->numberOfPoints(), test.numberOfPoints);
            EXPECT(!out.hasMissing());

            // the same as the named grid
            auto named = perform("{type: interpolate.grid2namedgrid, grid: O" + N + "}", in);

            EXPECT(out.representation()->sameAs(*named.representation()));
            EXPECT(out.values(0) == named.values(0));
        }
    }
}


CASE("interpolate.grid2octahedral-gg, values") {
    check_values(*make_checked("{type: interpolate.grid2octahedral-gg, octahedral: 8}"));
    check_values(*make_checked("{type: interpolate.grid2octahedral-gg, octahedral: 8, interpolation: nn}"),
                 "{grid: O16}", 3.5);
}


CASE("interpolate.grid2octahedral-gg, cropping") {
    auto a = make_mutable("{type: interpolate.grid2octahedral-gg, octahedral: 8}");

    EXPECT(!a->canCrop());

    EXPECT(a->mergeWithNext(*make("{type: crop.area, area: [60, -10, 30, 40]}")));
    EXPECT(a->canCrop());
    EXPECT(a->outputBoundingBox() == util::BoundingBox(60, -10, 30, 40));

    auto out = perform(*a, field("{grid: O16}", LATITUDE));

    EXPECT(0 < out.values(0).size() && out.values(0).size() < 544);
    EXPECT(contained(out, a->outputBoundingBox()));
}


CASE("interpolate.grid2octahedral-gg, properties") {
    auto a = make("{type: interpolate.grid2octahedral-gg, octahedral: 8}");

    EXPECT(a->isRegridAction());
    EXPECT(!a->canCrop());
    EXPECT_EQUAL(a->type(), "interpolate.grid2octahedral-gg");
    EXPECT_EQUAL(std::string(a->name()), "Gridded2OctahedralGG");
    EXPECT(a->outputBoundingBox() == util::BoundingBox(90, 0, -90, 360));
    EXPECT(a->json_str().find(R"("octahedral":8)") != std::string::npos);

    std::ostringstream print;
    print << *a;

    EXPECT(print.str().find("Gridded2OctahedralGG[N=8,interpolation=linear") == 0);
}


CASE("interpolate.grid2octahedral-gg, comparison") {
    auto a = make("{type: interpolate.grid2octahedral-gg, octahedral: 8}");

    EXPECT(*a == *make("{type: interpolate.grid2octahedral-gg, octahedral: 8, interpolation: linear}"));

    EXPECT(*a != *make("{type: interpolate.grid2octahedral-gg, octahedral: 16}"));
    EXPECT(*a != *make("{type: interpolate.grid2octahedral-gg, octahedral: 8, interpolation: nn}"));
    EXPECT(*a != *make("{type: interpolate.grid2namedgrid, grid: O8}"));
    EXPECT(*a != *make("{type: interpolate.grid2rotated-octahedral-gg, octahedral: 8, rotation: [-40, 22]}"));
    EXPECT(*a != *make("{type: interpolate.grid2regular-gg, regular: 8}"));
}


CASE("interpolate.grid2octahedral-gg, errors") {
    EXPECT_THROWS(make("interpolate.grid2octahedral-gg"));  // octahedral is required
}


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


CASE("interpolate.grid2reduced-gg") {
    struct test_t {
        size_t N;
        size_t numberOfPoints;
    };

    const std::vector<test_t> tests{test_t{32, 6114}};
    for (const auto& test : tests) {
        const auto N = std::to_string(test.N);

        SECTION("reduced: " + N) {
            auto a = make_checked("{type: interpolate.grid2reduced-gg, reduced: " + N + "}");

            std::string name;

            EXPECT(a->getGriddedTargetName(name));
            EXPECT_EQUAL(name, "N" + N);

            auto in  = field("{grid: O16}", LATITUDE, 2);
            auto out = perform(*a, in);

            EXPECT_EQUAL(out.dimensions(), 2);
            EXPECT_EQUAL(out.values(0).size(), test.numberOfPoints);
            EXPECT_EQUAL(out.representation()->numberOfPoints(), test.numberOfPoints);
            EXPECT(!out.hasMissing());

            // the same as the named grid
            auto named = perform("{type: interpolate.grid2namedgrid, grid: N" + N + "}", in);

            EXPECT(out.representation()->sameAs(*named.representation()));
            EXPECT(out.values(0) == named.values(0));
        }
    }
}


CASE("interpolate.grid2reduced-gg, values") {
    check_values(*make_checked("{type: interpolate.grid2reduced-gg, reduced: 32}"));
    check_values(*make_checked("{type: interpolate.grid2reduced-gg, reduced: 32, interpolation: nn}"), "{grid: O16}",
                 3.5);
}


CASE("interpolate.grid2reduced-gg, cropping") {
    auto a = make_mutable("{type: interpolate.grid2reduced-gg, reduced: 32}");

    EXPECT(!a->canCrop());

    EXPECT(a->mergeWithNext(*make("{type: crop.area, area: [60, -10, 30, 40]}")));
    EXPECT(a->canCrop());
    EXPECT(a->outputBoundingBox() == util::BoundingBox(60, -10, 30, 40));

    auto out = perform(*a, field("{grid: O16}", LATITUDE));

    EXPECT(0 < out.values(0).size() && out.values(0).size() < 6114);
    EXPECT(contained(out, a->outputBoundingBox()));
}


CASE("interpolate.grid2reduced-gg, properties") {
    auto a = make("{type: interpolate.grid2reduced-gg, reduced: 32}");

    EXPECT(a->isRegridAction());
    EXPECT(!a->canCrop());
    EXPECT_EQUAL(a->type(), "interpolate.grid2reduced-gg");
    EXPECT_EQUAL(std::string(a->name()), "Gridded2ReducedGG");
    EXPECT(a->outputBoundingBox() == util::BoundingBox(90, 0, -90, 360));
    EXPECT(a->json_str().find(R"("reduced":32)") != std::string::npos);

    std::ostringstream print;
    print << *a;

    EXPECT(print.str().find("Gridded2ReducedGG[N=32,interpolation=linear") == 0);
}


CASE("interpolate.grid2reduced-gg, comparison") {
    auto a = make("{type: interpolate.grid2reduced-gg, reduced: 32}");

    EXPECT(*a == *make("{type: interpolate.grid2reduced-gg, reduced: 32, interpolation: linear}"));

    EXPECT(*a != *make("{type: interpolate.grid2reduced-gg, reduced: 48}"));
    EXPECT(*a != *make("{type: interpolate.grid2reduced-gg, reduced: 32, interpolation: nn}"));
    EXPECT(*a != *make("{type: interpolate.grid2namedgrid, grid: N32}"));
    EXPECT(*a != *make("{type: interpolate.grid2rotated-reduced-gg, reduced: 32, rotation: [-40, 22]}"));
    EXPECT(*a != *make("{type: interpolate.grid2octahedral-gg, octahedral: 32}"));
}


CASE("interpolate.grid2reduced-gg, errors") {
    EXPECT_THROWS(make("interpolate.grid2reduced-gg"));  // reduced is required

    // N8 has no (classic) number of points per latitude
    EXPECT_THROWS(perform("{type: interpolate.grid2reduced-gg, reduced: 8}", field("{grid: O16}")));
}


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

    std::unique_ptr<repres::Iterator> it(out.representation()->iterator());
    std::unique_ptr<repres::Iterator> jt(unrotated.representation()->iterator());

    while (it->next()) {
        EXPECT(jt->next());
        EXPECT(same(it->pointUnrotated().lat().value(), jt->pointUnrotated().lat().value()));
        EXPECT(same(it->pointUnrotated().lon().value(), jt->pointUnrotated().lon().value()));
    }

    EXPECT(!jt->next());
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


CASE("interpolate.grid2reduced-gg-pl-given") {
    struct test_t {
        std::string pl;
        size_t numberOfPoints;
    };

    const std::vector<test_t> tests{
        test_t{"[20, 24, 24, 20]", 88},
        test_t{"[20, 24, 28, 32, 32, 28, 24, 20]", 208},
    };
    for (const auto& test : tests) {
        SECTION("pl: " + test.pl) {
            auto a = make_checked("{type: interpolate.grid2reduced-gg-pl-given, pl: " + test.pl + "}");

            std::string name;

            EXPECT(a->getGriddedTargetName(name));

            auto out = perform(*a, field("{grid: O16}", LATITUDE, 2));

            EXPECT_EQUAL(out.dimensions(), 2);
            EXPECT_EQUAL(out.values(0).size(), test.numberOfPoints);
            EXPECT_EQUAL(out.representation()->numberOfPoints(), test.numberOfPoints);
            EXPECT(!out.hasMissing());
        }
    }

    SECTION("the same as the octahedral grid") {
        // O2 has 20, 24, 24, 20 points per latitude
        auto in  = field("{grid: O16}", LATITUDE);
        auto out = perform("{type: interpolate.grid2reduced-gg-pl-given, pl: [20, 24, 24, 20]}", in);
        auto oct = perform("{type: interpolate.grid2octahedral-gg, octahedral: 2}", in);

        EXPECT_EQUAL(out.values(0).size(), oct.values(0).size());

        for (size_t i = 0; i < out.values(0).size(); ++i) {
            EXPECT(same(out.values(0)[i], oct.values(0)[i]));
        }
    }
}


CASE("interpolate.grid2reduced-gg-pl-given, values") {
    check_values(*make_checked("{type: interpolate.grid2reduced-gg-pl-given, pl: [20, 24, 28, 32, 32, 28, 24, 20]}"));
}


CASE("interpolate.grid2reduced-gg-pl-given, properties") {
    auto a = make("{type: interpolate.grid2reduced-gg-pl-given, pl: [20, 24, 24, 20]}");

    EXPECT(a->isRegridAction());
    EXPECT(!a->canCrop());
    EXPECT_EQUAL(a->type(), "interpolate.grid2reduced-gg-pl-given");
    EXPECT_EQUAL(std::string(a->name()), "Gridded2ReducedGGPLGiven");
    EXPECT(a->outputBoundingBox() == util::BoundingBox(90, 0, -90, 360));
    EXPECT(a->json_str().find(R"("pl":[20,24,24,20])") != std::string::npos);

    std::ostringstream print;
    print << *a;

    EXPECT(print.str().find("Gridded2ReducedGGPLGiven[pl=4,interpolation=linear") == 0);
}


CASE("interpolate.grid2reduced-gg-pl-given, comparison") {
    auto a = make("{type: interpolate.grid2reduced-gg-pl-given, pl: [20, 24, 24, 20]}");

    EXPECT(*a == *make("{type: interpolate.grid2reduced-gg-pl-given, pl: [20, 24, 24, 20], interpolation: linear}"));

    EXPECT(*a != *make("{type: interpolate.grid2reduced-gg-pl-given, pl: [20, 20, 20, 20]}"));
    EXPECT(*a != *make("{type: interpolate.grid2reduced-gg-pl-given, pl: [20, 24, 24, 20], interpolation: nn}"));
    EXPECT(*a != *make("{type: interpolate.grid2octahedral-gg, octahedral: 2}"));
    EXPECT(*a !=
           *make("{type: interpolate.grid2rotated-reduced-gg-pl-given, pl: [20, 24, 24, 20], rotation: [-40, 22]}"));
}


CASE("interpolate.grid2reduced-gg-pl-given, errors") {
    EXPECT_THROWS(make("interpolate.grid2reduced-gg-pl-given"));  // pl is required
}


const std::string ROTATED_REDUCED_GG_PL_GIVEN =
    "{type: interpolate.grid2rotated-reduced-gg-pl-given, pl: [20, 24, 28, 32, 32, 28, 24, 20], rotation: [-40, 22]";


CASE("interpolate.grid2rotated-reduced-gg-pl-given") {
    auto a   = make_checked(ROTATED_REDUCED_GG_PL_GIVEN + "}");
    auto in  = field("{grid: O16}", LATITUDE, 2);
    auto out = perform(*a, in);

    EXPECT_EQUAL(out.dimensions(), 2);
    EXPECT_EQUAL(out.values(0).size(), 208);
    EXPECT_EQUAL(out.representation()->numberOfPoints(), 208);
    EXPECT(out.representation()->uniqueName().find("R4-") == 0);
    EXPECT(out.representation()->uniqueName().find("-rot:-40:22:0") != std::string::npos);
    EXPECT(!out.hasMissing());

    // the same grid, unrotated, has the same points in the rotated frame
    auto unrotated = perform("{type: interpolate.grid2reduced-gg-pl-given, pl: [20, 24, 28, 32, 32, 28, 24, 20]}", in);

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


CASE("interpolate.grid2rotated-reduced-gg-pl-given, values") {
    check_values(*make_checked(ROTATED_REDUCED_GG_PL_GIVEN + "}"));
    check_values(*make_checked(ROTATED_REDUCED_GG_PL_GIVEN + ", interpolation: nn}"), "{grid: O16}", 3.5);
}


CASE("interpolate.grid2rotated-reduced-gg-pl-given, cropping") {
    auto a = make_mutable(ROTATED_REDUCED_GG_PL_GIVEN + "}");

    EXPECT(!a->canCrop());
    EXPECT(a->outputBoundingBox() == util::BoundingBox(90, 0, -90, 360));

    // cropping in the rotated frame (centred on the rotated equator/Greenwich), bounding box (unrotated) containing it
    EXPECT(a->mergeWithNext(*make("{type: crop.area, area: [20, -20, -20, 20]}")));
    EXPECT(a->canCrop());
    EXPECT(!(a->outputBoundingBox() == util::BoundingBox(90, 0, -90, 360)));
    EXPECT(a->outputBoundingBox().contains(PointLatLon(50, 22)));

    auto out = perform(*a, field("{grid: O16}", LATITUDE));

    EXPECT(0 < out.values(0).size() && out.values(0).size() < 208);
    EXPECT(contained(out, a->outputBoundingBox()));
}


CASE("interpolate.grid2rotated-reduced-gg-pl-given, properties") {
    auto a = make(ROTATED_REDUCED_GG_PL_GIVEN + "}");

    EXPECT(a->isRegridAction());
    EXPECT(!a->canCrop());

    std::string name;

    EXPECT(!a->getGriddedTargetName(name));

    EXPECT_EQUAL(a->type(), "interpolate.grid2rotated-reduced-gg-pl-given");
    EXPECT_EQUAL(std::string(a->name()), "Gridded2RotatedReducedGGPLGiven");
    EXPECT(a->json_str().find(R"("rotation":[-40,22],"pl":[20,24,28,32,32,28,24,20])") != std::string::npos);

    std::ostringstream print;
    print << *a;

    EXPECT(print.str().find("Gridded2RotatedReducedGGPLGiven[pl=8,rotation=") == 0);
    EXPECT(print.str().find("interpolation=linear") != std::string::npos);
}


CASE("interpolate.grid2rotated-reduced-gg-pl-given, comparison") {
    auto a = make(ROTATED_REDUCED_GG_PL_GIVEN + "}");

    EXPECT(*a == *make(ROTATED_REDUCED_GG_PL_GIVEN + ", interpolation: linear}"));
    EXPECT(*a == *make("{type: interpolate.grid2rotated-reduced-gg-pl-given, pl: [20, 24, 28, 32, 32, 28, 24, 20], "
                       "rotation: -40/22}"));

    EXPECT(*a != *make("{type: interpolate.grid2rotated-reduced-gg-pl-given, pl: [20, 24, 28, 32, 32, 28, 24, 20], "
                       "rotation: [-40, 23]}"));
    EXPECT(*a !=
           *make("{type: interpolate.grid2rotated-reduced-gg-pl-given, pl: [20, 24, 24, 20], rotation: [-40, 22]}"));
    EXPECT(*a != *make(ROTATED_REDUCED_GG_PL_GIVEN + ", interpolation: nn}"));
    EXPECT(*a != *make("{type: interpolate.grid2reduced-gg-pl-given, pl: [20, 24, 28, 32, 32, 28, 24, 20]}"));
    EXPECT(*a != *make("{type: interpolate.grid2rotated-octahedral-gg, octahedral: 4, rotation: [-40, 22]}"));
}


CASE("interpolate.grid2rotated-reduced-gg-pl-given, errors") {
    // rotation is required
    EXPECT_THROWS(make("{type: interpolate.grid2rotated-reduced-gg-pl-given, pl: [20, 24, 28, 32, 32, 28, 24, 20]}"));

    EXPECT_THROWS(make("{type: interpolate.grid2rotated-reduced-gg-pl-given, rotation: [-40, 22]}"));
    EXPECT_THROWS(
        make("{type: interpolate.grid2rotated-reduced-gg-pl-given, pl: [20, 24, 28, 32, 32, 28, 24, 20], rotation: "
             "[-40, 22, 0]}"));
    EXPECT_THROWS(make("{type: interpolate.grid2rotated-reduced-gg-pl-given, pl: [], rotation: [-40, 22]}"));
}


}  // namespace mir::action::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
