/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 *
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include <algorithm>
#include <memory>
#include <numeric>
#include <vector>

#include "eckit/testing/Test.h"
#include "eckit/types/FloatCompare.h"

#include "mir/api/MIRJob.h"
#include "mir/api/mir_config.h"
#include "mir/data/MIRField.h"
#include "mir/input/GribFileInput.h"
#include "mir/input/GribMemoryInput.h"
#include "mir/input/RawInput.h"
#include "mir/key/grid/Grid.h"
#include "mir/output/GribMemoryOutput.h"
#include "mir/output/RawOutput.h"
#include "mir/repres/Representation.h"
#include "mir/util/Angles.h"
#include "mir/util/Atlas.h"
#include "mir/util/BoundingBox.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"
#include "mir/util/Types.h"

// define EXPECTV(a) log << "\tEXPECT(" << #a <<")" << std::endl; EXPECT(a)


namespace mir::tests::unit {


#if 0
CASE("BoundingBox") {
    using util::BoundingBox;

    auto& log = Log::info();
    auto old  = log.precision(16);

    const std::vector<BoundingBox> boxes{
        {90, 0, -90, 360},
        {90, 0, -90, 720},
        {90, 360, -90, 720},
        {90, -360, -90, 360},
        {90, -180, -90, 180},
        {90, 180, -90, -180},
        {90, 0, -90, 356},
        {90, 0, -90, 358},
        {90, 0, -90, 359.5},
        {90, 0, -90, 359},
        {90, 0, -90, 359.999},
        {90, -180, -90, 179.99},
        {90, 0, -90, 359.99},
        {90, 0, -90, 359.9},
        {90, 0, -90, 359.6489},
        {90, -350, -10, 9},
        {90, 10, -10, 9},
        {89.7626, -114.8915, -88.2374, 243.1085},
        {89.7626, -114.8907, -88.2374, 243.1093},
        {89, 1, -89, 359},
        {88, -178, -88, 180},
        {85, 0, -90, 357},
        {71.8, -10.66, 34.56, 32.6},
        {70.9, -40.987, 19.73, 40},
        {59.9531, 23, 35.0722, 80},
        {58.5, -6.1, 36, 20.7},
        {57.9852, 230, 25.0918, 300},
        {51.9406, 7.00599, 43.0847, 27.6923},
        {51.941, 7.005, 43.084, 27.693},
        {43.9281, 91, 21.0152, 143},
        {40, 50, -50, 169.532},
        {37.6025, -114.8915, 27.7626, -105.188},
        {37.6025, -114.8915, 27.7626, -105.1875},
        {37.6025, -114.8907, 27.7626, -105.1875},
        {37.6025, -114.891, 27.7626, -105.188},
        {37.575, -114.892, 27.803, -105.187},
        {37.5747, 245.109, 27.8032, 254.812},
        {36.345879, 113.586968, 35.816463, 114.420328},
        {36.34501645, 113.58806225, 35.81244723, 114.41866527},
        {35.00112, 112.9995593, 33.990671, 114.0092641},
        {34.657355, 113.04832, 34.29085525, 113.74528},
        {34.6548026, 113.04894614, 34.2911562, 113.747272},
        {27.9, 253, 27.8, 254},
        {11.8782, 279, -49.9727, 325},
        {3, 1, 1, 3},
        {2.1, 0, 0, 2.1},
        {2, 0, 0, 2},
        {2, 0, -2, 360},
        {0, -350, 0, 9},
        {0, -350, 0, 8},
        {0, 0, 0, 2},
        {0, 0, 0, 0},
        {-10, -85, -39, -56.1},
        {-10.0176, 275, -38.9807, 304},
        {-10.017, -85, -38.981, -56},
        {-25.0918, 135, -46.8801, 179},
    };

    SECTION("operator==") {
        std::vector<Longitude> _delta{Longitude::GLOBE * -2, Longitude::GLOBE * -1, Longitude::GREENWICH,
                                      Longitude::GLOBE, Longitude::GLOBE * 2};

        for (const auto& A : boxes) {
            for (auto delta : _delta) {
                auto B = BoundingBox(A.north(), A.west() + delta, A.south(), A.east() + delta);

                static size_t c = 1;
                log << "Test " << c++ << ":"
                    << "\n\t"
                       "d="
                    << delta
                    << "\n\t"
                       "A="
                    << A << " (empty? " << A.empty() << ")"
                    << "\n\t"
                       "B="
                    << B << " (empty? " << B.empty() << ")" << std::endl;

                EXPECT(A == A);
                if (delta == Longitude::GREENWICH) {
                    EXPECT(A == B);
                }
                else {
                    EXPECT(A != B);
                }
            }
        }
    }

    SECTION("intersects (combinations)") {
        for (const auto& A : boxes) {
            for (const auto& B : boxes) {

                auto AiB         = B;
                auto BiA         = A;
                bool commutative = A.intersects(AiB) == B.intersects(BiA);

                static size_t c = 1;
                log << "Test " << c++ << ":"
                    << "\n\t"
                       "A="
                    << A << " (empty? " << A.empty() << ")"
                    << "\n\t"
                       "B="
                    << B << " (empty? " << B.empty() << ")"
                    << "\n\t"
                       "A intersects B = (empty? "
                    << AiB.empty() << ") = " << AiB
                    << "\n\t"
                       "B intersects A = (empty? "
                    << BiA.empty() << ") = " << BiA << std::endl;

                EXPECT(commutative);
                if (A.isPeriodicWestEast() && B.isPeriodicWestEast()) {
                    EXPECT(AiB.isPeriodicWestEast() && BiA.isPeriodicWestEast());
                    EXPECT(AiB.west() == B.west());
                    EXPECT(BiA.west() == A.west());
                }
                else {
                    EXPECT(AiB == BiA && AiB.west() == BiA.west());
                }

                if (A.empty() || B.empty()) {
                    EXPECT(AiB.empty());
                }

                if (!AiB.empty()) {
                    EXPECT(A.contains(AiB));
                    EXPECT(B.contains(AiB));
                }

                if (A == B) {
                    EXPECT(A == AiB);
                    EXPECT(A.contains(B));
                }

                if (A.isPeriodicWestEast()) {
                    EXPECT(B.isPeriodicWestEast() == AiB.isPeriodicWestEast());
                    if (!B.isPeriodicWestEast()) {
                        EXPECT(B.east() == AiB.east());
                        EXPECT(B.west() == AiB.west());
                    }
                }

                if (B.isPeriodicWestEast()) {
                    EXPECT(A.isPeriodicWestEast() == AiB.isPeriodicWestEast());
                    if (!A.isPeriodicWestEast()) {
                        EXPECT(A.east() == AiB.east());
                        EXPECT(A.west() == AiB.west());
                    }
                }

                if (!AiB.empty()) {
                    if (A.isPeriodicWestEast() != B.isPeriodicWestEast()) {
                        BoundingBox Awe = {AiB.north(), A.west(), AiB.south(), A.east()};
                        BoundingBox Bwe = {AiB.north(), B.west(), AiB.south(), B.east()};
                        EXPECT(AiB == Awe || AiB == Bwe);
                    }

                    const std::vector<Longitude> lims{
                        {A.west()},
                        {A.east()},
                        {B.west()},
                        {B.east()},
                    };
                    auto n = std::count_if(lims.begin(), lims.end(), [&AiB](const Longitude& lon) {
                        return AiB.contains(AiB.north(), lon) || AiB.contains(AiB.south(), lon);
                    });
                    EXPECT(2 <= n && n <= 4);
                }
            }
        }
    }

    SECTION("intersects (point)") {
        std::vector<Latitude> _lat{-90, -89, -88, 2, 1, 0, 1, 2, 88, 89, 90};
        std::vector<Longitude> _lon{
            -360, -358, -182, -180, -178, -2, 0, 2, 178, 180, 182, 358, 360, 362, 718, 720, 722,
        };

        for (const auto& A : boxes) {
            for (Latitude lat : _lat) {
                for (Longitude lon : _lon) {

                    const BoundingBox P{lat, lon, lat, lon};
                    ASSERT(P.empty());

                    auto AiP        = P;
                    bool intersects = A.intersects(AiP);

                    static size_t c = 1;
                    log << "Test " << c++ << ":"
                        << "\n\t"
                           "A="
                        << A << " (empty? " << A.empty() << ")"
                        << "\n\t"
                           "P="
                        << P << " (empty? " << P.empty() << ")"
                        << "\n\t"
                           "A intersects P = (intersects? "
                        << intersects << ", empty? " << AiP.empty() << ") = " << AiP << std::endl;

                    EXPECT(intersects == A.contains(P));
                    if (intersects) {
                        EXPECT(P.contains(AiP));
                    }
                }
            }
        }
    }

    SECTION("intersects (periodic longitude ranges)") {
        std::vector<double> _shifts{-180, -90, -2, -1, 0, 1, 2, 90, 180};
        for (auto shift : _shifts) {
            const BoundingBox A;
            ASSERT(A.isPeriodicWestEast());

            const BoundingBox B{A.north(), A.west() + shift, A.south(), A.east() + shift};
            EXPECT(B.isPeriodicWestEast());

            BoundingBox AiB(B);
            bool AintersectsB = A.intersects(AiB);
            bool AiBequalsB   = AiB == B;

            BoundingBox BiA(A);
            bool BintersectsA = B.intersects(BiA);
            bool BiAequalsA   = BiA == A;

            static size_t c = 1;
            log << "Test " << c++ << ":"
                << "\n\t"
                   "A="
                << A
                << "\n\t"
                   "B="
                << B
                << "\n\t"
                   "A intersects B = "
                << AiB << " = B ? " << AiBequalsB
                << "\n\t"
                   "B intersects A = "
                << BiA << " = A ? " << BiAequalsA << std::endl;

            EXPECT(AintersectsB);
            EXPECT(AiBequalsB);
            EXPECT(BintersectsA);
            EXPECT(BiAequalsA);
        }
    }

    log.precision(old);
}


CASE("Representation::extendBoundingBox") {
    using key::grid::Grid;
    using util::BoundingBox;

    auto& log = Log::info();
    auto old  = log.precision(16);

    SECTION("Gaussian") {

        std::vector<std::string> _name{"F16", "O16", "F21", "O21"};
#if mir_HAVE_ATLAS
        _name.emplace_back("N16");
#endif

        std::vector<BoundingBox> _bbox{{90, 10, -10, 9}, {90, -350, -10, 9}, {-70, -10, -90, 10},
                                       {0, -350, 0, 9},  {60, -350, 20, 9},  {90, -10, 70, 10}};

        for (const auto& name : _name) {
            repres::RepresentationHandle repres = Grid::lookup(name).representation();

            for (const auto& bbox : _bbox) {
                BoundingBox extended = repres->extendBoundingBox(bbox);

                log << name << "\t > " << *repres << " + extendBoundingBox(" << "\n\t   " << bbox << ")"
                    << "\n\t = " << extended << std::endl;

                EXPECT(extended.contains(bbox));
            }
        }
    }

    log.precision(old);
}


CASE("IFS climate files") {
    std::unique_ptr<input::MIRInput> in(new input::GribFileInput("orog_1km.grib2"));
    ASSERT(in->next());

    util::BoundingBox bbox(in->parametrisation());

    for (const auto& test : {
             std::make_pair(bbox.north().fraction(), eckit::Fraction(21599, 240)),
             std::make_pair(bbox.west().fraction(), eckit::Fraction(1, 240)),
             std::make_pair(bbox.south().fraction(), eckit::Fraction(-21599, 240)),
             std::make_pair(bbox.east().fraction(), eckit::Fraction(86399, 240)),
         }) {
        Log::info() << test.first << " == " << test.second << std::endl;
        EXPECT_EQUAL(test.first, test.second);
    }
}
#endif


#if 0
CASE("Gaussian grid bounding box") {
    // input
    param::SimpleParametrisation meta1;
    meta1.set("gridded", true);
    meta1.set("gridType", "reduced_gg");
    meta1.set("north", 90.);
    meta1.set("west", 0.);
    meta1.set("south", -90.);
    meta1.set("east", 360.);

    std::vector<long> pl{20, 24, 28, 32, 36, 40, 44, 48, 48, 44, 40, 36, 32, 28, 24, 20};
    ASSERT(pl.size() % 2 == 0);

    meta1.set("pl", pl);
    meta1.set("N", pl.size() / 2);

    std::vector<double> values1(std::accumulate(pl.begin(), pl.end(), static_cast<size_t>(0)), 0.);
    std::unique_ptr<input::MIRInput> input(new input::RawInput(values1.data(), values1.size(), meta1));


    // output (rough memory estimate for the F8 grid)
    // std::vector<char> output_data(/*header*/ 1024 + /*data*/ sizeof(double) * 512, 0.);
    param::SimpleParametrisation meta2;
    std::vector<double> values2(512, 0.);

    std::unique_ptr<output::MIROutput> output(new output::RawOutput(values2.data(), values2.size(), meta2));


    // job (no plan compression)
    api::MIRJob job;
    job.set("grid", "F8");
    job.set("area", std::vector<double>{90., -180., -90., 180.});  // West-East periodic


    auto get_area = [](const param::MIRParametrisation& param) -> std::vector<double> {
        std::vector<double> area(4);
        ASSERT(param.get("latitudeOfFirstGridPointInDegrees", area[0]));
        ASSERT(param.get("longitudeOfFirstGridPointInDegrees", area[1]));
        ASSERT(param.get("latitudeOfLastGridPointInDegrees", area[2]));
        ASSERT(param.get("longitudeOfLastGridPointInDegrees", area[3]));
        return area;
    };


    auto check_area = [](const std::vector<double>& area1, const std::vector<double>& area2) -> bool {
        constexpr double EPS = 1e-3;

        // normalise because of GRIB2
        return eckit::types::is_approximately_equal(area1[0], area2[0], EPS) &&
               eckit::types::is_approximately_equal(util::normalise_longitude(area1[1], 0),
                                                    util::normalise_longitude(area2[1], 0), EPS) &&
               eckit::types::is_approximately_equal(area1[2], area2[2], EPS) &&
               eckit::types::is_approximately_equal(util::normalise_longitude(area1[3], 0),
                                                    util::normalise_longitude(area2[3], 0), EPS);
    };


    SECTION("dont-compress-plan: False") {
        // process
        job.set("dont-compress-plan", false /*default*/);

        job.execute(*input, *output);

        std::unique_ptr<input::MIRInput> encoded_input(
            new input::GribMemoryInput(values2.data(), values2.size()));
        const auto& encoded = encoded_input->parametrisation();

        std::string grid;
        EXPECT(encoded.get("gridName", grid) && grid == "F8");

        const auto area = get_area(encoded);
        EXPECT(check_area({81.651, -180., -81.651, 168.75}, area));
    }


#if 0
    SECTION("dont-compress-plan: True") {
        // process
        job.set("dont-compress-plan", true);

        job.execute(*input, *output);

        std::unique_ptr<input::MIRInput> encoded_input(
            new input::GribMemoryInput(output_data.data(), output_data.size()));
        const auto& encoded = encoded_input->parametrisation();

        std::string grid;
        EXPECT(encoded.get("gridName", grid) && grid == "F8");

        const auto area = get_area(encoded);
        EXPECT(check_area({81.651, -180., -81.651, 168.75}, area));
    }
#endif
}
#endif


#if mir_HAVE_ATLAS
#if ATLAS_HAVE_PROJ
CASE("Polar stereographic grid bounding box") {
    std::unique_ptr<input::MIRInput> in(new input::GribFileInput("gridType=polar_stereographic.grib2"));
    ASSERT(in->next());

    auto field                          = in->field();
    repres::RepresentationHandle repres = field.representation();
    auto bbox                           = repres->boundingBox();
    std::cout << bbox << std::endl;
}
#endif
#endif


}  // namespace mir::tests::unit


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
