// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <sstream>
#include <vector>

#include "interpolate.h"


namespace mir::action::tests {


CASE("interpolate.grid2regular-ll") {
    struct test_t {
        std::string options;
        size_t numberOfPoints;
        util::BoundingBox bbox;
    };

    auto in = field("{grid: O16}", LATITUDE, 2);

    const std::vector<test_t> tests{
        test_t{"grid: [10, 10]", 19 * 36, {90, 0, -90, 350}},
        test_t{"grid: 10/10", 19 * 36, {90, 0, -90, 350}},
        test_t{"grid: [20, 20]", 9 * 18, {80, 0, -80, 340}},  // globalised, not including the poles
        test_t{"grid: [2, 2], area: [60, -10, 30.5, 40]", 90 * 180, {88.5, 0, -89.5, 358}},  // shifted
        test_t{"grid: [2, 2], cropping: [60, -10, 30, 40]", 16 * 26, {60, -10, 30, 40}},
    };
    for (const auto& test : tests) {
        SECTION(test.options) {
            auto out = perform("{type: interpolate.grid2regular-ll, " + test.options + "}", in);

            EXPECT_EQUAL(out.dimensions(), 2);
            EXPECT_EQUAL(out.values(0).size(), test.numberOfPoints);
            EXPECT_EQUAL(out.values(1).size(), test.numberOfPoints);
            EXPECT_EQUAL(out.representation()->numberOfPoints(), test.numberOfPoints);
            EXPECT(out.representation()->boundingBox() == test.bbox);
            EXPECT(!out.hasMissing());
        }
    }
}


CASE("interpolate.grid2regular-ll, values") {
    SECTION("constant") {
        auto out = perform("{type: interpolate.grid2regular-ll, grid: [10, 10]}", field("{grid: O16}", CONSTANT));

        EXPECT(max_error(out, CONSTANT) < 1e-12);
    }

    SECTION("linear in latitude") {
        auto out = perform("{type: interpolate.grid2regular-ll, grid: [10, 10]}", field("{grid: O16}", LATITUDE, 2));

        // within the input latitudes (O16 first latitude is 85.76)
        EXPECT(max_error(out, LATITUDE, 80.) < 0.2);
        EXPECT(out.values(0) == out.values(1));
    }

    SECTION("constant, linear in latitude, and missing values") {
        check_values(*make_checked("{type: interpolate.grid2regular-ll, grid: [10, 10]}"));
        check_values(*make_checked("{type: interpolate.grid2regular-ll, grid: [10, 10]}"), "{grid: [5, 5]}", 1e-9, 90.);
    }

    SECTION("the same grid") {
        auto in  = field("{grid: [10, 10]}", LATITUDE);
        auto out = perform("{type: interpolate.grid2regular-ll, grid: [10, 10]}", in);

        EXPECT(max_error(out, LATITUDE) < 1e-9);
    }
}


CASE("interpolate.grid2regular-ll, missing values") {
    auto in = field("{grid: O16}", MISSING_NORTH, 1, 9999.);

    EXPECT(in.hasMissing());

    const auto north = [](double lat) { return lat > 50.; };
    const auto south = [](double lat) { return lat < 40.; };

    SECTION("default (missing-if-heaviest-missing)") {
        auto out = perform("{type: interpolate.grid2regular-ll, grid: [10, 10]}", in);

        EXPECT(out.hasMissing());
        EXPECT_EQUAL(out.missingValue(), 9999.);

        const auto n = count_missing_if(out, north);

        EXPECT(n.points > 0 && n.missing == n.points);

        const auto s = count_missing_if(out, south);

        EXPECT(s.points > 0 && s.missing == 0);

        EXPECT(max_error(out, LATITUDE, 40.) < 0.2);
    }

    SECTION("non-linear treatments") {
        auto count = [&in](const std::string& nonLinear) {
            return count_missing(
                perform("{type: interpolate.grid2regular-ll, grid: [2, 2], non-linear: " + nonLinear + "}", in));
        };

        const auto any      = count("missing-if-any-missing");
        const auto heaviest = count("missing-if-heaviest-missing");
        const auto all      = count("missing-if-all-missing");

        EXPECT(any > heaviest);
        EXPECT(heaviest > all);
        EXPECT(all > 0);
    }

    SECTION("no missing values") {
        auto out = perform("{type: interpolate.grid2regular-ll, grid: [10, 10]}", field("{grid: O16}", LATITUDE));

        EXPECT(!out.hasMissing());
        EXPECT_EQUAL(count_missing(out), 0);
    }
}


CASE("interpolate.grid2regular-ll, interpolation methods") {
    struct test_t {
        std::string interpolation;
        double tolerance;       // to a field linear in latitude
        bool everywhere;        // all output points have a value
        bool missing;           // missing values are propagated (Atlas methods are not aware of missing values)
        bool comparable;        // by spec (data-dependent methods are never the same)
        bool roundtrip = true;  // false if the complete spec does not build the same action (known issue)
    };

    // O16 latitude spacing is about 5.5 degrees, output grid boxes are 10 degrees
    const std::vector<test_t> tests{
        test_t{"nn", 3.5, true, true, true},
        test_t{"nearest-neighbour", 3.5, true, true, true},
        test_t{"linear", 0.2, true, true, true},
        test_t{"bilinear", 0.2, true, true, true},
        test_t{"k-nearest", 3.5, true, true, true},
        test_t{"k-nearest, nearest-method: nclosest", 1.5, true, true, true},
        test_t{"k-nearest, nearest-method: nclosest-or-nearest", 1.5, true, true, true},
        test_t{"k-nearest, nearest-method: longest-element-diagonal-and-nclosest", 1.5, true, true, true},
        test_t{"k-nearest, nearest-method: sample, distance: 1000000", 9., true, true, true},
        test_t{"k-nearest, nearest-method: sorted-sample, distance: 1000000", 9., true, true, false},
        test_t{"k-nearest, nearest-method: distance, distance: 300000", 3., false, true, true},
        test_t{"k-nearest, nearest-method: nclosest, distance-weighting: inverse-distance-weighting", 1.5, true, true,
               true},
        test_t{"k-nearest, nearest-method: nclosest, distance-weighting: shepard", 1.5, true, true, true, false},
        test_t{"k-nearest, nearest-method: nclosest, distance-weighting: reciprocal", 1.5, true, true, true, false},
        test_t{"k-nearest, nearest-method: nclosest, distance-weighting: no-distance-weighting", 3., true, true, true},
        test_t{"k-nearest, nearest-method: nclosest, distance-weighting: nearest-neighbour", 3.5, true, true, true},
        test_t{"k-nearest, nearest-method: nclosest, distance-weighting: pseudo-laplace", 3., true, true, true},
        test_t{"k-nearest, nearest-method: nclosest, distance-weighting: gaussian, "
               "distance-weighting-gaussian-stddev: 100000",
               3.5, true, true, true},
        test_t{"k-nearest, nearest-method: nclosest, distance-weighting: cressman, distance: 1000000, "
               "cressman-model-extension-power: 2",
               3., true, true, true},
        test_t{"k-nearest, nearest-method: nclosest, distance-weighting: climate-filter, distance: 1000000", 3., true,
               true, true, false},
        test_t{"k-nearest-statistics", 8., true, true, false},
        test_t{"k-nearest-statistics, interpolation-statistics: mean", 4., true, true, false, false},
        test_t{"k-nearest-statistics, interpolation-statistics: minimum", 8., true, true, false, false},
        test_t{"grid-box-average", 2., true, true, true},
        test_t{"grid-box-statistics", 5., false, true, false},
        test_t{"grid-box-statistics, interpolation-statistics: minimum", 5., false, true, false},
        test_t{"grid-box-statistics, interpolation-statistics: mean", 5., false, true, false},
        test_t{"voronoi-statistics", 6., true, true, false},
        test_t{"voronoi-statistics, interpolation-statistics: minimum", 6., true, true, false},
        test_t{"restructure", 6., false, true, true},
        test_t{"grid-box-average-matrix-free", 2., true, false, true},
        test_t{"grid-box-maximum-matrix-free", 5., true, false, true},
        test_t{"structured-bilinear", 1e-6, true, false, true},
        test_t{"structured-bicubic", 1e-6, true, false, true},
        test_t{"structured-biquasicubic", 1e-6, true, false, true},
    };
    for (const auto& test : tests) {
        SECTION(test.interpolation) {
            auto a =
                make("{type: interpolate.grid2regular-ll, grid: [10, 10], interpolation: " + test.interpolation + "}");

            if (test.roundtrip) {
                auto b = make(a->json_str(false));

                EXPECT_EQUAL(b->json_str(false), a->json_str(false));

                if (test.comparable) {
                    EXPECT(*a == *b);
                }
            }

            auto constant = perform(*a, field("{grid: O16}", CONSTANT));

            EXPECT(max_error(constant, CONSTANT) < 1e-9);
            EXPECT(!test.everywhere || !constant.hasMissing());

            auto linear = perform(*a, field("{grid: O16}", LATITUDE));

            EXPECT(max_error(linear, LATITUDE, 80.) <= test.tolerance);

            if (test.missing) {
                auto in  = field("{grid: O16}", MISSING_NORTH, 1, 9999.);
                auto out = perform(*a, in);

                EXPECT(out.hasMissing());

                const auto n = count_missing_if(out, [](double lat) { return lat > 55.; });

                EXPECT(n.points > 0 && n.missing == n.points);

                if (test.everywhere) {
                    const auto s = count_missing_if(out, [](double lat) { return lat < 35.; });

                    EXPECT(s.points > 0 && s.missing == 0);
                }
            }
        }
    }
}


CASE("interpolate.grid2regular-ll, cropping") {
    auto in = field("{grid: O16}", LATITUDE);

    auto check_cropped = [](const data::MIRField& out) {
        EXPECT_EQUAL(out.values(0).size(), 16 * 26);
        EXPECT(out.representation()->boundingBox() == util::BoundingBox(60, -10, 30, 40));
        EXPECT(contained(out, util::BoundingBox(60, -10, 30, 40)));
        EXPECT(max_error(out, LATITUDE) < 0.2);
    };

    SECTION("cropping (spec)") {
        auto a = make_checked("{type: interpolate.grid2regular-ll, grid: [2, 2], cropping: [60, -10, 30, 40]}");

        EXPECT(a->canCrop());
        EXPECT(a->outputBoundingBox() == util::BoundingBox(60, -10, 30, 40));

        check_cropped(perform(*a, in));
    }

    const std::vector<std::string> interpolations{"linear", "nn", "grid-box-average", "structured-bilinear"};
    for (const std::string& interpolation : interpolations) {
        SECTION("merged with crop.area, interpolation: " + interpolation) {
            const auto spec = "{type: interpolate.grid2regular-ll, grid: [2, 2], interpolation: " + interpolation;

            auto a = make_mutable(spec + "}");

            EXPECT(!a->canCrop());
            EXPECT(a->outputBoundingBox() == util::BoundingBox(90, 0, -90, 360));

            EXPECT(!a->mergeWithNext(*make("{type: mask.area, area: [60, -10, 30, 40]}")));
            EXPECT(!a->mergeWithNext(*make("{type: interpolate.grid2regular-ll, grid: [1, 1]}")));
            EXPECT(!a->canCrop());

            EXPECT(a->mergeWithNext(*make("{type: crop.area, area: [60, -10, 30, 40]}")));
            EXPECT(a->canCrop());
            EXPECT(a->outputBoundingBox() == util::BoundingBox(60, -10, 30, 40));

            // the same as by spec
            auto b = make(spec + ", cropping: [60, -10, 30, 40]}");

            EXPECT(*a == *b);
            EXPECT_EQUAL(a->json_str(false), b->json_str(false));

            if (interpolation != "nn") {
                EXPECT(*a != *make(spec + "}"));
            }

            // a cropping interpolation can be merged into a previous one
            EXPECT(make_mutable("{type: interpolate.grid2regular-ll, grid: [1, 1]}")->mergeWithNext(*a));

            auto out = perform(*a, in);

            EXPECT_EQUAL(out.values(0).size(), 16 * 26);
            EXPECT(out.representation()->boundingBox() == util::BoundingBox(60, -10, 30, 40));
        }
    }

    SECTION("not merged (method does not crop)") {
        auto a = make_mutable("{type: interpolate.grid2regular-ll, grid: [2, 2], interpolation: fail}");

        EXPECT(!a->mergeWithNext(*make("{type: crop.area, area: [60, -10, 30, 40]}")));
        EXPECT(!a->canCrop());
    }

    SECTION("global cropping, aligned with the input (no cropping)") {
        auto out = perform("{type: interpolate.grid2regular-ll, grid: [2, 2], cropping: [90, 0, -90, 360]}", in);

        EXPECT_EQUAL(out.values(0).size(), 91 * 180);
        EXPECT(out.representation()->boundingBox() == util::BoundingBox(90, 0, -90, 358));
    }

    SECTION("global cropping, not aligned with the input (renumbering)") {
        auto out = perform("{type: interpolate.grid2regular-ll, grid: [2, 2], cropping: [90, -180, -90, 180]}", in);

        EXPECT_EQUAL(out.values(0).size(), 91 * 180);
        EXPECT(out.representation()->boundingBox() == util::BoundingBox(90, -180, -90, 178));

        std::unique_ptr<repres::Iterator> it(out.representation()->iterator());

        EXPECT(it->next());
        EXPECT(same(it->pointUnrotated().lon().value(), -180.));
    }

    SECTION("regional input") {
        auto regional = field("{grid: [1, 1], area: [60, -10, 30, 40]}", LATITUDE);

        // output is cropped to the input
        check_cropped(perform("{type: interpolate.grid2regular-ll, grid: [2, 2]}", regional));
        check_cropped(
            perform("{type: interpolate.grid2regular-ll, grid: [2, 2], interpolation-global-output: true}", regional));

        // input as if global: output is global, missing values outside of the input
        auto out =
            perform("{type: interpolate.grid2regular-ll, grid: [2, 2], interpolation-global-input: true}", regional);
        EXPECT_EQUAL(out.values(0).size(), 91 * 180);
        EXPECT(out.hasMissing());
        EXPECT_EQUAL(out.values(0).size() - count_missing(out), 16 * 26);
    }
}


CASE("interpolate.grid2regular-ll, properties") {
    auto a = make("{type: interpolate.grid2regular-ll, grid: [2, 2]}");

    EXPECT(a->isRegridAction());
    EXPECT(!a->isCropAction());
    EXPECT(!a->isEndAction());
    EXPECT(!a->canCrop());

    std::string name;

    EXPECT(!a->getGriddedTargetName(name));

    EXPECT_EQUAL(a->type(), "interpolate.grid2regular-ll");
    EXPECT_EQUAL(std::string(a->name()), "Gridded2RegularLL");
    EXPECT(a->outputBoundingBox() == util::BoundingBox(90, 0, -90, 360));

    std::ostringstream print;
    print << *a;

    EXPECT(print.str().find("Gridded2RegularLL[increments=") == 0);
    EXPECT(print.str().find("interpolation=linear") != std::string::npos);

    std::ostringstream custom;
    a->custom(custom);

    EXPECT_EQUAL(custom.str(), print.str());

    // the interpolation method spec, flattened
    const auto json = a->json_str();

    EXPECT(json.find(R"("interpolation":"linear")") != std::string::npos);
    EXPECT(json.find(R"("solver":{"type":"multiply"})") != std::string::npos);
    EXPECT(json.find(R"("grid":[2,2],"area":[90,0,-90,358])") != std::string::npos);
    EXPECT(json.find("cropping") == std::string::npos);
    EXPECT(json.find("interpolation-global") == std::string::npos);

    const std::vector<std::string> options{"interpolation-global-input", "interpolation-global-output"};
    for (const std::string& option : options) {
        auto b = make_checked("{type: interpolate.grid2regular-ll, grid: [2, 2], " + option + ": true}");

        EXPECT(b->json_str().find('"' + option + R"(":true)") != std::string::npos);
    }
}


CASE("interpolate.grid2regular-ll, comparison") {
    auto a = make_checked("{type: interpolate.grid2regular-ll, grid: [2, 2]}");

    EXPECT(*a == *make("{type: interpolate.grid2regular-ll, grid: 2/2}"));
    EXPECT(*a == *make("{type: interpolate.grid2regular-ll, grid: [2, 2], interpolation: linear}"));
    EXPECT(*a == *make("{type: interpolate.grid2regular-ll, grid: [2, 2], area: [90, 0, -90, 358]}"));

    const std::vector<std::string> others{
        "{type: interpolate.grid2regular-ll, grid: [1, 1]}",
        "{type: interpolate.grid2regular-ll, grid: [2, 2], area: [60, -10, 30.5, 40]}",
        "{type: interpolate.grid2regular-ll, grid: [2, 2], interpolation: nn}",
        "{type: interpolate.grid2regular-ll, grid: [2, 2], non-linear: missing-if-all-missing}",
        "{type: interpolate.grid2regular-ll, grid: [2, 2], cropping: [60, -10, 30, 40]}",
        "{type: interpolate.grid2rotated-regular-ll, grid: [2, 2], rotation: [-90, 0]}",
        "{type: interpolate.grid2regular-gg, regular: 45}",
        "{type: crop.area, area: [90, 0, -90, 358]}",
    };
    for (const std::string& other : others) {
        EXPECT(*a != *make(other));
    }

    auto b = make("{type: interpolate.grid2regular-ll, grid: [2, 2], interpolation: nn}");

    EXPECT(
        *b !=
        *make("{type: interpolate.grid2regular-ll, grid: [2, 2], interpolation: k-nearest, nearest-method: nclosest}"));
    EXPECT(*b != *make("{type: interpolate.grid2regular-ll, grid: [2, 2], interpolation: k-nearest}"));
}


CASE("interpolate.grid2regular-ll, errors") {
    EXPECT_THROWS(make("interpolate.grid2regular-ll"));                           // grid is required
    EXPECT_THROWS(make("{type: interpolate.grid2regular-ll, grid: [2, 2, 2]}"));  // grid [we, sn]
    EXPECT_THROWS(make("{type: interpolate.grid2regular-ll, grid: [2, 2], cropping: [1, 2, 3]}"));
    EXPECT_THROWS(make("{type: interpolate.grid2regular-ll, grid: [2, 2], interpolation: unknown}"));

    // method failing intentionally
    EXPECT_THROWS(
        perform("{type: interpolate.grid2regular-ll, grid: [2, 2], interpolation: fail}", field("{grid: O16}")));
}


const std::string ROTATION = "rotation: [-40, 22]";


CASE("interpolate.grid2rotated-regular-ll") {
    struct test_t {
        std::string options;
        size_t numberOfPoints;
        util::BoundingBox bbox;  // in the rotated frame
    };

    const std::vector<test_t> tests{
        test_t{"grid: [10, 10]", 19 * 36, {90, 0, -90, 350}},
        test_t{"grid: [20, 20]", 9 * 18, {80, 0, -80, 340}},
        test_t{"grid: [2, 2], area: [10, -10, -9, 10]", 90 * 180, {89, 0, -89, 358}},  // shifted
        test_t{"grid: [2, 2], cropping: [10, -10, -10, 10]", 11 * 11, {10, -10, -10, 10}},
    };
    for (const auto& test : tests) {
        SECTION(test.options) {
            auto a =
                make_checked("{type: interpolate.grid2rotated-regular-ll, " + test.options + ", " + ROTATION + "}");
            auto out = perform(*a, field("{grid: O16}", LATITUDE, 2));

            EXPECT_EQUAL(out.dimensions(), 2);
            EXPECT_EQUAL(out.values(0).size(), test.numberOfPoints);
            EXPECT_EQUAL(out.representation()->numberOfPoints(), test.numberOfPoints);
            EXPECT(out.representation()->boundingBox() == test.bbox);
            EXPECT(out.representation()->uniqueName().find("-rot:-40:22:0") != std::string::npos);
            EXPECT(!out.hasMissing());
            EXPECT(contained(out, a->outputBoundingBox()));
        }
    }

    SECTION("rotated (south) pole") {
        auto out = perform("{type: interpolate.grid2rotated-regular-ll, grid: [10, 10], " + ROTATION + "}",
                           field("{grid: O16}", LATITUDE));

        size_t n = 0;

        for (std::unique_ptr<repres::Iterator> it(out.representation()->iterator()); it->next();) {
            if (it->pointUnrotated().lat() == Latitude::SOUTH_POLE) {
                const auto& p = *(*it);

                EXPECT(same(p[0], -40., 1e-6));
                EXPECT(same(p[1], 22., 1e-6));
                EXPECT(std::abs(out.values(0)[it->index()] - -40.) < 0.2);

                ++n;
            }
        }

        EXPECT(n > 0);
    }
}


CASE("interpolate.grid2rotated-regular-ll, values") {
    check_values(*make_checked("{type: interpolate.grid2rotated-regular-ll, grid: [10, 10], " + ROTATION + "}"));
    check_values(*make_checked("{type: interpolate.grid2rotated-regular-ll, grid: [10, 10], " + ROTATION +
                               ", interpolation: nn}"),
                 "{grid: O16}", 3.5);
}


CASE("interpolate.grid2rotated-regular-ll, cropping") {
    auto a = make_mutable("{type: interpolate.grid2rotated-regular-ll, grid: [2, 2], " + ROTATION + "}");

    EXPECT(!a->canCrop());
    EXPECT(a->outputBoundingBox() == util::BoundingBox(90, 0, -90, 360));

    // cropping in the rotated frame, bounding box (unrotated) containing it
    EXPECT(a->mergeWithNext(*make("{type: crop.area, area: [10, -10, -10, 10]}")));
    EXPECT(a->canCrop());

    const auto bbox = a->outputBoundingBox();

    EXPECT(!(bbox == util::BoundingBox(10, -10, -10, 10)));
    EXPECT(util::BoundingBox(61, 2, 39, 42).contains(bbox));
    EXPECT(bbox.contains(util::BoundingBox(59, 3, 40, 41)));

    auto b = make("{type: interpolate.grid2rotated-regular-ll, grid: [2, 2], " + ROTATION +
                  ", cropping: [10, -10, -10, 10]}");
    EXPECT(*a == *b);
    EXPECT(b->outputBoundingBox() == bbox);

    auto out = perform(*a, field("{grid: O16}", LATITUDE));

    EXPECT_EQUAL(out.values(0).size(), 11 * 11);
    EXPECT(contained(out, bbox));
    EXPECT(max_error(out, LATITUDE) < 0.2);
}


CASE("interpolate.grid2rotated-regular-ll, properties") {
    auto a = make("{type: interpolate.grid2rotated-regular-ll, grid: [2, 2], " + ROTATION + "}");

    EXPECT(a->isRegridAction());
    EXPECT(!a->canCrop());

    std::string name;

    EXPECT(!a->getGriddedTargetName(name));

    EXPECT_EQUAL(a->type(), "interpolate.grid2rotated-regular-ll");
    EXPECT_EQUAL(std::string(a->name()), "Gridded2RotatedLL");

    const auto json = a->json_str();

    EXPECT(json.find(R"("rotation":[-40,22],"grid":[2,2],"area":[90,0,-90,358])") != std::string::npos);

    std::ostringstream print;
    print << *a;

    EXPECT(print.str().find("Gridded2RotatedLL[increments=") == 0);
    EXPECT(print.str().find(",rotation=") != std::string::npos);
    EXPECT(print.str().find("interpolation=linear") != std::string::npos);
}


CASE("interpolate.grid2rotated-regular-ll, comparison") {
    auto a = make("{type: interpolate.grid2rotated-regular-ll, grid: [2, 2], " + ROTATION + "}");

    EXPECT(*a == *make("{type: interpolate.grid2rotated-regular-ll, grid: 2/2, rotation: -40/22}"));
    EXPECT(*a ==
           *make("{type: interpolate.grid2rotated-regular-ll, grid: [2, 2], " + ROTATION + ", interpolation: linear}"));

    const std::vector<std::string> others{
        "{type: interpolate.grid2rotated-regular-ll, grid: [2, 2], rotation: [-40, 23]}",
        "{type: interpolate.grid2rotated-regular-ll, grid: [2, 2], rotation: [-41, 22]}",
        "{type: interpolate.grid2rotated-regular-ll, grid: [1, 1], rotation: [-40, 22]}",
        "{type: interpolate.grid2rotated-regular-ll, grid: [2, 2], rotation: [-40, 22], area: [10, -10, -9, 10]}",
        "{type: interpolate.grid2rotated-regular-ll, grid: [2, 2], rotation: [-40, 22], interpolation: nn}",
        "{type: interpolate.grid2regular-ll, grid: [2, 2]}",
        "{type: interpolate.grid2rotated-regular-gg, regular: 45, rotation: [-40, 22]}",
    };
    for (const std::string& other : others) {
        EXPECT(*a != *make(other));
    }
}


CASE("interpolate.grid2rotated-regular-ll, errors") {
    EXPECT_THROWS(make("{type: interpolate.grid2rotated-regular-ll, " + ROTATION + "}"));  // grid is required
    EXPECT_THROWS(make("{type: interpolate.grid2rotated-regular-ll, grid: [2, 2]}"));      // rotation is required
    EXPECT_THROWS(make("{type: interpolate.grid2rotated-regular-ll, grid: [2, 2, 2], " + ROTATION + "}"));
    EXPECT_THROWS(make("{type: interpolate.grid2rotated-regular-ll, grid: [2, 2], rotation: [-40, 22, 0]}"));
}


// the action is registered, but its output representation (and comparison) is not implemented


CASE("interpolate.grid2reduced-ll, properties") {
    auto a = make("interpolate.grid2reduced-ll");

    EXPECT(a->isRegridAction());
    EXPECT(!a->canCrop());

    std::string name;

    EXPECT(!a->getGriddedTargetName(name));

    EXPECT_EQUAL(a->type(), "interpolate.grid2reduced-ll");
    EXPECT_EQUAL(std::string(a->name()), "Gridded2ReducedLL");
    EXPECT(a->json_str().find(R"({"type":"interpolate.grid2reduced-ll","interpolation":"linear")") == 0);

    std::ostringstream print;
    print << *a;

    EXPECT(print.str().find("Gridded2ReducedLL[interpolation=linear") == 0);
}


CASE("interpolate.grid2reduced-ll, errors") {
    EXPECT_THROWS(make("{type: interpolate.grid2reduced-ll, interpolation: unknown}"));

    auto a = make("interpolate.grid2reduced-ll");

    EXPECT_THROWS(a->outputBoundingBox());
    EXPECT_THROWS(perform(*a, field("{grid: O16}")));

    auto b = make("interpolate.grid2reduced-ll");

    EXPECT_THROWS(static_cast<void>(*a == *b));
}


// the action is registered, but its output representation (and comparison) is not implemented


CASE("interpolate.grid2reduced-ll-pl-given, properties") {
    auto a = make("interpolate.grid2reduced-ll-pl-given");

    EXPECT(a->isRegridAction());
    EXPECT(!a->canCrop());

    std::string name;

    EXPECT(!a->getGriddedTargetName(name));

    EXPECT_EQUAL(a->type(), "interpolate.grid2reduced-ll-pl-given");
    EXPECT_EQUAL(std::string(a->name()), "Gridded2ReducedLLPLGiven");
    EXPECT(a->json_str().find(R"({"type":"interpolate.grid2reduced-ll-pl-given","interpolation":"linear")") == 0);

    std::ostringstream print;
    print << *a;

    EXPECT(print.str().find("Gridded2ReducedLLPLGiven[interpolation=linear") == 0);
}


CASE("interpolate.grid2reduced-ll-pl-given, errors") {
    EXPECT_THROWS(make("{type: interpolate.grid2reduced-ll-pl-given, interpolation: unknown}"));

    auto a = make("interpolate.grid2reduced-ll-pl-given");

    EXPECT_THROWS(a->outputBoundingBox());
    EXPECT_THROWS(perform(*a, field("{grid: O16}")));

    auto b = make("interpolate.grid2reduced-ll-pl-given");

    EXPECT_THROWS(static_cast<void>(*a == *b));
}


}  // namespace mir::action::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
