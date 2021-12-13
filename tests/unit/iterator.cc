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


#include <ios>

#include "eckit/testing/Test.h"
#include "eckit/types/Fraction.h"

#include "mir/iterator/detail/RegularIterator.h"
#include "mir/key/grid/Grid.h"
#include "mir/repres/Representation.h"
#include "mir/repres/gauss/GaussianIterator.h"
#include "mir/util/BoundingBox.h"
#include "mir/util/Increments.h"
#include "mir/util/Log.h"


namespace mir {
namespace tests {
namespace unit {


using eckit::Fraction;
using util::BoundingBox;
using util::Increments;

static auto& log = Log::info();


#define EXPECTV(a)                                \
    log << "\tEXPECT(" << #a << ")" << std::endl; \
    EXPECT(a)


CASE("MIR-390") {
    auto old(log.precision(16));
    log << std::boolalpha;

    // 89.9958333333, 0.004166666667, -89.9958333333, 359.995833333
    BoundingBox bbox{Fraction(21599, 240), Fraction(1, 240), Fraction(-21599, 240), Fraction(86355, 240)};
    PointLatLon reference{bbox.south(), bbox.west()};

    size_t Ni = 43200;
    size_t Nj = 21600;

    Increments inc((bbox.east().fraction() - bbox.west().fraction()) / Ni,
                   (bbox.north().fraction() - bbox.south().fraction()) / Nj);


    SECTION("LatLon::correctBoundingBox") {

        using iterator::detail::RegularIterator;

        // Latitude/longitude ranges
        RegularIterator lat{bbox.south().fraction(), bbox.north().fraction(), inc.south_north().latitude().fraction(),
                            reference.lat().fraction()};

        auto n = lat.b();
        auto s = lat.a();

        EXPECTV(bbox.south().fraction() == s);
        EXPECTV(bbox.north().fraction() == n);
        EXPECTV(Nj = lat.n());

        RegularIterator lon{bbox.west().fraction(), bbox.east().fraction(), inc.west_east().longitude().fraction(),
                            reference.lon().fraction(), Longitude::GLOBE.fraction()};

        auto w = lon.a();
        auto e = lon.b();

        EXPECTV(bbox.west().fraction() == w);
        EXPECTV(bbox.east().fraction() == e);
        EXPECTV(Ni = lon.n());

        // checks
        ASSERT(w + (Ni - 1) * lon.inc() == e || Ni * lon.inc() == Longitude::GLOBE.fraction());
        ASSERT(s + (Nj - 1) * lat.inc() == n);

        EXPECTV(bbox == BoundingBox(n, w, s, e));
    }


    log.precision(old);
}


CASE("GaussianIterator") {
    auto old(log.precision(16));

    struct test_t {
        const std::string grid;
        size_t numberOfPoints;
        size_t numberOfCroppedPoints;
        BoundingBox bbox;
        Point2 globalP1;
        Point2 globalP2;
        Point2 localP1;
        Point2 localP2;
    };

    BoundingBox globe;


    SECTION("GaussianIterator") {
        std::vector<test_t> tests{
        {"F128", 131072, 858, {80.351, -150.46875, 79.648, 150.46875}, {89.462821568577,0},{89.462821568577,0.703125},{80.350237152036,-150.46875},{80.350237152036,-149.765625}},
        {"F128", 131072, 429, {80.350, -150.46875, 79.648, 150.46875}, {89.462821568577,0},{89.462821568577,0.703125},{79.648526993666,-150.46875},{79.648526993666,-149.765625}},
        {"F320", 819200, 912, {56, -16.5, 50.5, -3}, {89.784876907219,0},{89.784876907219,0.28125},{55.784517651898,-16.3125},{55.784517651898,-16.03125}},
        {"F1280", 13107200, 910080, {75, -30, 25, 60}, {89.946187715666,0},{89.946187715666,0.0703125},{74.97363389192699,-29.953125},{74.97363389192699,-29.8828125}},
        {"O320", 421120, 16133, {58, -130, 25, -60},{89.784876907219,0},{89.784876907219,18},{57.751728769309,-129.327731092437},{57.751728769309,-128.5714285714286}},
        {"O320", 421120, 7511, {-25, 135, -47, 179},{89.784876907219,0},{89.784876907219,18},{-25.152216627037,135.1914893617021},{-25.152216627037,135.5744680851064}},
        {"O320", 421120, 28101, {12, -81, -50, -35},{89.784876907219,0},{89.784876907219,18},{11.943790218098,-80.74468085106383},{11.943790218098,-80.42553191489361}},
        {"O320", 421120, 9877, {44, 91, 21, 143},{89.784876907219,0},{89.784876907219,18},{43.981247789094,91.07142857142857},{43.981247789094,91.60714285714286}},
        {"O320", 421120, 8743, {60, 23, 35, 80},{89.784876907219,0},{89.784876907219,18},{59.999969762058,23.51351351351351},{59.999969762058,24.32432432432432}},
        {"O640", 1661440, 4512, {52, 7, 43, 27.7},{89.89239644558999,0},{89.89239644558999,18},{51.940642605563,7.2},{51.940642605563,7.527272727272727}},
        {"O640", 1661440, 63842, {58, -130, 25, -60},{89.89239644558999,0},{89.89239644558999,18},{57.985155059998,-129.9568965517241},{57.985155059998,-129.5689655172414}},
        {"O640", 1661440, 29674, {-25, 135, -47, 179},{89.89239644558999,0},{89.89239644558999,18},{-25.091758985537,135},{-25.091758985537,135.1931330472103}},
        {"O1280", 6599680, 442460, {12, -81, -50, -35},{89.946187715666,0},{89.946187715666,18},{11.985940014128,-80.95152603231598},{11.985940014128,-80.87073608617594}},
        {"O1280", 6599680, 12370, {37.6025, -114.8907, 27.7626, -105.1875},{89.946187715666,0},{89.946187715666,18},{37.574691602387,-114.84},{37.574691602387,-114.72}},
        {"O1280", 6599680, 6599646, {90, 0, -90, 359.929},{89.946187715666,0},{89.946187715666,18},{89.946187715666,0},{89.946187715666,18}},
        {"O1280", 6599680, 118210, {-25, 135, -47, 179},{89.946187715666,0},{89.946187715666,18},{-25.061510912726,135},{-25.061510912726,135.0969827586207}},
        {"N128", 88838, 88838, {90, 0, -90, 359.297},{89.462821568577,0},{89.462821568577,20},{89.462821568577,0},{89.462821568577,20}},
        {"N320", 542080, 13437, {-60, 50, -90, 180},{89.784876907219,0},{89.784876907219,20},{-60.280999861572,50.13333333333333},{-60.280999861572,50.66666666666666}},
        {"N640", 2140702, 52654, {-60, 50, -90, 180},{89.89239644558999,0},{89.89239644558999,20},{-60.093705799122,50.13333333333333},{-60.093705799122,50.4}},
        };

        for (const auto& t : tests) {
            log << "Test " + t.grid + " (global)" << std::endl;
            repres::RepresentationHandle global(key::grid::Grid::lookup(t.grid).representation());
            std::unique_ptr<repres::Iterator> git{global->iterator()};

            EXPECT(global->numberOfPoints() == t.numberOfPoints);

            ASSERT(git->next());
            EXPECT(globe.contains(*(*git)));
            EXPECT(eckit::geometry::points_equal(*(*git), t.globalP1));

            ASSERT(git->next());
            EXPECT(globe.contains(*(*git)));
            EXPECT(eckit::geometry::points_equal(*(*git), t.globalP2));

            log << "Test " + t.grid + " (cropped)" << std::endl;
            repres::RepresentationHandle cropped(global->croppedRepresentation(t.bbox));
            std::unique_ptr<repres::Iterator> cit{cropped->iterator()};

            EXPECT(cropped->numberOfPoints() == t.numberOfCroppedPoints);

            ASSERT(cit->next());
            EXPECT(t.bbox.contains(*(*cit)));
            EXPECT(eckit::geometry::points_equal(*(*cit), t.localP1));

            ASSERT(cit->next());
            EXPECT(t.bbox.contains(*(*cit)));
            EXPECT(eckit::geometry::points_equal(*(*cit), t.localP2));
        }
    }


    SECTION("MIR-555") {
        test_t t{"F640", 3276800, 2400, {54., -16.5, 50.5, -3.}, {89.89239644558999, 0.}, {89.89239644558999, 0.140625}, {53.908623449839, -16.453125}, {53.908623449839, -16.3125}};

        log << "Test " + t.grid + " (global)" << std::endl;
        repres::RepresentationHandle global(key::grid::Grid::lookup(t.grid).representation());
        std::unique_ptr<repres::Iterator> git{global->iterator()};

        EXPECT(global->numberOfPoints() == t.numberOfPoints);

        ASSERT(git->next());
        EXPECT(globe.contains(*(*git)));
        EXPECT(eckit::geometry::points_equal(*(*git), t.globalP1));

        ASSERT(git->next());
        EXPECT(globe.contains(*(*git)));
        EXPECT(eckit::geometry::points_equal(*(*git), t.globalP2));

        log << "Test " + t.grid + " (cropped)" << std::endl;
        repres::RepresentationHandle cropped(global->croppedRepresentation(t.bbox));
        std::unique_ptr<repres::Iterator> cit{cropped->iterator()};

        EXPECT(cropped->numberOfPoints() == t.numberOfCroppedPoints);

        ASSERT(cit->next());
        EXPECT(t.bbox.contains(*(*cit)));
        EXPECT(eckit::geometry::points_equal(*(*cit), t.localP1));

        ASSERT(cit->next());
        EXPECT(t.bbox.contains(*(*cit)));
        EXPECT(eckit::geometry::points_equal(*(*cit), t.localP2));
    }


    log.precision(old);
}

}  // namespace unit
}  // namespace tests
}  // namespace mir


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
