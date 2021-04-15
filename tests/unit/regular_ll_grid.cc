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


#include <cmath>
#include <memory>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

#include "eckit/testing/Test.h"
#include "eckit/types/FloatCompare.h"
#include "eckit/types/Fraction.h"

#include "mir/data/MIRField.h"
#include "mir/input/GribFileInput.h"
#include "mir/repres/latlon/RegularLL.h"
#include "mir/util/BoundingBox.h"
#include "mir/util/Domain.h"
#include "mir/util/Increments.h"
#include "mir/util/Types.h"


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


CASE("MIR-351") {

    auto old(log.precision(16));
    log << std::boolalpha;

    using eckit::types::is_approximately_equal;

    // setup this case exact values
    auto round_to_precision = [](double value, double precision) { return std::round(value * precision) / precision; };

    // MIR-351: there is a problem in the encoding of latitudes, however
    // it doesn't trigger any known malfunction
    //    Fraction sn(30, 128);
    //    auto n(341 * sn + sn / 2);
    //    auto s(-n);

    //    const double north = round_to_precision(n, 1000000);
    //    const double south = round_to_precision(s, 1000000);

    Fraction we(45, 128);
    Fraction w(we / 2);
    Fraction e(360 - we / 2);

    const double west = round_to_precision(w, 1000000);
    const double east = round_to_precision(e, 1000000);

    log << "Reference values:"
        << "\n\t"
           "west  = "
        << west
        << "\n\t"
           "east  = "
        << east << std::endl;

    {
        // file wrongly encoded: fails EXPECTV(domain.isPeriodicWestEast())
        // const std::string file = "MIR-351.wrong.grib2";

        const std::string file = "MIR-351.corrected.grib2";
        log << "Reading '" << file << "'" << std::endl;

        std::unique_ptr<input::MIRInput> input(new input::GribFileInput(file));
        ASSERT(input->next());

        auto& param = input->parametrisation();

        double encoded_west = 0.;
        ASSERT(param.get("west", encoded_west));
        EXPECTV(is_approximately_equal(encoded_west, west));

        double encoded_east = 0.;
        ASSERT(param.get("east", encoded_east));
        EXPECTV(is_approximately_equal(encoded_east, east));

        repres::RepresentationHandle repres(input->field().representation());

        // West remains the same value, but East can be corrected
        auto bbox(repres->boundingBox());
        log << "\t" << bbox << std::endl;
        EXPECTV(is_approximately_equal(west, bbox.west().value()));

        auto domain(repres->domain());
        log << "\t" << domain << std::endl;
        EXPECTV(!domain.isGlobal());
        EXPECTV(domain.isPeriodicWestEast());
    }


    log.precision(old);
}


#if 0
CASE("Increments::correctBoundingBox") {

    auto old(log.precision(16));
    log << std::boolalpha;


    SECTION("basic") {
        auto test = [](bool shiftLon, bool shiftLat, size_t Ni, size_t Nj) {
            Increments inc { 1, 1 };
            BoundingBox box;
            PointLatLon ref { shiftLat ? 0.5 : 0., shiftLon ? 0.5 : 0. };

            repres::RepresentationHandle repres(new repres::latlon::RegularLL(inc, box, ref));
            auto& ll = dynamic_cast<const repres::latlon::RegularLL&>(*repres);

            static size_t c = 1;
            log << "Test " << c++ << ":"
                << "\n\t" "   RegularLL(increments=" << inc << ", reference=" << ref << ")"
                << "\n\t" " = " << ll
                << std::endl;

            EXPECT(ll.Ni() == Ni);
            EXPECT(ll.Nj() == Nj);

            auto& bbox = repres->boundingBox();
            EXPECT(bbox.west() == ref.lon());
            EXPECT(bbox.east() == bbox.west() + (ll.Ni() - 1));
            EXPECT(bbox.north() == bbox.south() + (ll.Nj() - 1));

            repres->validate(MIRValuesVector(Ni * Nj));
        };

        test(false, false, 360, 181);
        test(true,  false, 360, 181);
        test(false, true,  360, 180);
        test(true,  true,  360, 180);
    }


    SECTION("small areas") {
        Increments inc(3, 3);

        for (auto& box : std::vector<BoundingBox>{
            { 4, -1, -1,  4},
            {12, -1, -1, 12},
        }) {

            for (auto& reference : std::vector<PointLatLon>{
                    { 0,  0 },
                    { 0, -1 },
                    {-1,  0 },
                    { 1, -1 },
                    { 0,  1 },
                    { 1,  0 },
                    { 1,  1 },
                }) {

                auto sn = inc.south_north().latitude();
                auto we = inc.west_east().longitude();
                bool expectLatitudeShift = !(reference.lat().fraction() / sn.fraction()).integer();
                bool expectLongitudeShift = !(reference.lon().fraction() / we.fraction()).integer();

                auto corrected(box);
                size_t ni;
                size_t nj;
                repres::latlon::LatLon::correctBoundingBox(corrected, ni, nj, inc, reference);

                static size_t c = 1;
                log << "Test " << c++ << ":"
                    << "\n\t   " << inc
                    << "\n\t + " << box
                    << "\n\t > " << corrected << " corrected with reference " << reference
                    << "\n\t > ni = " << ni
                    << "\n\t > nj = " << nj
                    << "\n\t = shifted in latitude? " << inc.isLatitudeShifted(corrected)
                    << "\n\t = shifted in longitude? " << inc.isLongitudeShifted(corrected)
                    << std::endl;

                EXPECT(box.contains(corrected));
                EXPECT(ni >= 1);
                EXPECT(nj >= 1);

                EXPECT(corrected.west() == reference.lon());
                EXPECT(corrected.west() + (ni - 1) * we.fraction() == corrected.east());
                EXPECT(corrected.west() + ni * we.fraction() > box.east());

                EXPECT(corrected.south() == reference.lat());

                EXPECT(corrected.south() + (nj - 1) * sn.fraction() == corrected.north());
                EXPECT(corrected.south() + nj * sn.fraction() > box.north());


                EXPECT(corrected.south() + (nj - 1) * sn.fraction() == corrected.north());
                EXPECT(corrected.south() + nj * sn.fraction() > box.north());

                EXPECT(expectLatitudeShift  || !inc.isLatitudeShifted(corrected));
                EXPECT(expectLongitudeShift || !inc.isLongitudeShifted(corrected));
            }
        }
    }


    SECTION("equator") {
        BoundingBox equator(0, -1, 0, 359);

        for (auto& inc : {
                Increments{ 3, 3 },
                Increments{ 7, 0 },
            }) {

            for (auto& reference : std::vector<PointLatLon>{
                    { 0,  0 },
                    { 0, -1 },
                    {-1,  0 },
                    { 1, -1 },
                    { 0,  1 },
                    { 1,  0 },
                    { 1,  1 },
                }) {

                auto sn = inc.south_north().latitude();
                auto we = inc.west_east().longitude();

                auto corrected = equator;
                size_t ni;
                size_t nj;
                repres::latlon::LatLon::correctBoundingBox(corrected, ni, nj, inc, reference);

                static size_t c = 1;
                log << "Test " << c++ << ":"
                    << "\n\t   " << inc
                    << "\n\t + " << equator
                    << "\n\t > " << corrected << " corrected with reference " << reference
                    << "\n\t > ni = " << ni
                    << "\n\t > nj = " << nj
                    << "\n\t = shifted in latitude? " << inc.isLatitudeShifted(corrected)
                    << "\n\t = shifted in longitude? " << inc.isLongitudeShifted(corrected)
                    << std::endl;

                if (reference.lat() == 0) {
                    EXPECT(equator.contains(corrected));
                }
                EXPECT(ni >= 1);

                if (sn == 0) {
                    EXPECT(corrected.south() == equator.south());
                }

                EXPECT(corrected.south() == corrected.north());
                EXPECT(corrected.west() == reference.lon());

                if (inc.isPeriodic()) {
                    EXPECT(ni * we.fraction() == Longitude::GLOBE.fraction());
                } else {
                    auto Nabove = ni;
                    auto Nbelow = Nabove - 1;
                    EXPECT(Nabove * we.fraction() > Longitude::GLOBE.fraction());
                    EXPECT(Nbelow * we.fraction() < Longitude::GLOBE.fraction());
                }
            }
        }
    }


    log.precision(old);
}









































struct Case {
    Case(std::string&& name, const Increments& increments, const BoundingBox& boundingBox,
         const BoundingBox& correctBoundingBox, bool allowLatitudeShift, bool allowLongitudeShift,
         size_t ni, size_t nj) :
        name_(name),
        increments_(increments),
        boundingBox_(boundingBox),
        correctBoundingBox_(correctBoundingBox),
        ni_(ni),
        nj_(nj),
        allowLatitudeShift_(allowLatitudeShift),
        allowLongitudeShift_(allowLongitudeShift) {}

    const std::string name_;
    const Increments increments_;
    const BoundingBox boundingBox_;
    const BoundingBox correctBoundingBox_;
    const size_t ni_;
    const size_t nj_;
    const bool allowLatitudeShift_;
    const bool allowLongitudeShift_;

    bool compare(const Case& other) const {
        if (boundingBox_ != other.boundingBox_ || ni_ != other.ni_ || nj_ != other.nj_) {
            log << "\n\t"
                   "   "
                << *this
                << "\n\t"
                   "!= "
                << other << std::endl;
            return false;
        }
        return true;
    }


    friend std::ostream& operator<<(std::ostream& s, const Case& c) {
        s << "Case[name=" << c.name_ << ",increments=" << c.increments_
          << ",boundingBox=" << c.boundingBox_ << ",ni=" << c.ni_ << ",nj=" << c.nj_ << "]";
        return s;
    }
};


CASE("x") {
    auto old(log.precision(16));
    log << std::boolalpha;

    BoundingBox GLOBE;
    BoundingBox FIX;

    Increments inc[] = {
        Increments{0, 0}, Increments{1, 1}, Increments{2, 2}, Increments{3, 3},
        Increments{4, 4}, Increments{5, 5}, Increments{6, 6}, Increments{7, 7},
    };

    BoundingBox box00(-0.2, -0.3, -0.2, -0.3);
    BoundingBox box11(0.9, -0.2, -0.1, 0.8);
    BoundingBox box22(1, -1, -1, 1);
    BoundingBox box33(2, -1, -1, 2);


    SECTION("LatLon::LatLon(Increments, BoundingBox):") {
        for (const auto& kase : {
                 Case("001", inc[7], {85, 0, -90, 357}, FIX, true, true, 52, 26),
                 Case("002", inc[4], {90, 0, -90, 356}, FIX, true, true, 90, 46),
                 Case("003", inc[2], {2, 0, 0, 2}, FIX, true, true, 2, 2),
                 Case("004", inc[2], {2.1, 0, 0, 2.1}, FIX, true, true, 2, 2),
                 Case("005", inc[2], {3, 1, 1, 3}, FIX, true, true, 2, 2),
                 Case("006", inc[2], {37.6025, -114.8907, 27.7626, -105.1875}, FIX, true, true, 5,
                      5),
                 Case("007", inc[2], {37.6025, -114.8907, 27.7626, -105.188}, FIX, true, true, 5,
                      5),
                 Case("008", inc[2], {88, -178, -88, 180}, FIX, true, true, 180, 89),
                 Case("009", inc[1], {0, -350, 0, 8}, FIX, true, true, 359, 1),
                 Case("010", Increments(0.5, 0.5), {0, -350, 0, 9}, FIX, true, true, 719, 1),

                 Case("011", inc[7], {85, 0, -90, 357}, FIX, true, true, 52, 26),
                 Case("012", inc[7], {85, 0, -90, 357}, FIX, true, true, 52, 26),
                 Case("013", inc[7], GLOBE, FIX, true, true, 52, 26),
                 Case("014", inc[4], {90, 0, -90, 356}, FIX, true, true, 90, 46),
                 Case("015", inc[4], {90, 0, -90, 356}, FIX, true, true, 90, 46),
                 Case("016", inc[4], GLOBE, FIX, true, true, 90, 46),
                 Case("017", inc[2], {90, 0, -90, 358}, FIX, true, true, 180, 91),
                 Case("018", inc[2], {90, 0, -90, 358}, FIX, true, true, 180, 91),
                 Case("019", inc[2], {89, 1, -89, 359}, FIX, true, true, 180, 90),
                 Case("020", inc[2], {90, 0, -90, 358}, FIX, true, true, 180, 91),
                 Case("021", inc[2], {89.7626, 1.1093, -88.2374, 359.1093}, FIX, true, true, 180,
                      90),
                 Case("022", inc[2], {89.7626, 1.1093, -88.2374, 359.1093}, FIX, true, true, 180,
                      90),
                 Case("023", inc[1], {90, 0, -90, 359}, FIX, true, true, 360, 181),
                 Case("024", Increments(0.5, 0.5), {90, 0, -90, 359.5}, FIX, true, true, 720, 361),

                 Case("MIR-251.001", Increments{0.003474259, 0.003496601747573},
                      {34.6548026, 113.04894614, 34.2911562, 113.7472724}, FIX, true, true, 0, 0),
                 Case("MIR-251.002", Increments{0.003474259, 0.003496601747573},
                      {34.6548026, 113.04894614, 34.2911562, 113.747272}, FIX, true, true, 0, 0),
                 Case("MIR-251.003", Increments{0.00352, 0.003558252427184},
                      {34.657355, 113.04832, 34.29085525, 113.74528}, FIX, true, true, 0, 0),
                 Case("MIR-251.004", Increments{0.006198529411765, 0.005665625},
                      {36.34501645, 113.58806225, 35.81244723, 114.41866527}, FIX, true, true, 0,
                      0),
                 Case("MIR-251.005", Increments{0.0097087, 0.010417},
                      {35.00112, 112.9995593, 33.990671, 114.0092641}, FIX, true, true, 0, 0),
                 Case("MIR-251.006", Increments{0.010417, 0.007353},
                      {36.345879, 113.586968, 35.816463, 114.420328}, FIX, true, true, 0, 0),
                 Case("MIR-251.007", Increments{0.166667, 0.166667},
                      {51.166769, 10.833355, 42.500085, 28.166723}, FIX, true, true, 0, 0),
                 Case("MIR-251.008", Increments{0.2999999, 0.3},
                      {-5.7, 105.8999647, -6, 106.1999646}, FIX, true, true, 0, 0),


                 Case("MIR-309.001", inc[0], box00, box00, false, false, 0, 0),
                 Case("MIR-309.002", inc[0], box00, box00, false, true, 0, 0),
                 Case("MIR-309.003", inc[0], box00, box00, true, false, 0, 0),
                 Case("MIR-309.004", inc[0], box00, box00, true, true, 0, 0),

                 Case("MIR-309.005", inc[1], box00, {0, 0, 0, 0}, false, false, 0, 0),
                 Case("MIR-309.006", inc[1], box00, {0, box00.west(), 0, box00.west()}, false, true,
                      0, 0),
                 Case("MIR-309.007", inc[1], box00, {box00.south(), 0, box00.south(), 0}, true,
                      false, 0, 0),
                 Case("MIR-309.008", inc[1], box00, box00, true, true, 0, 0),

                 Case("MIR-309.009", inc[2], box00, {0, 0, 0, 0}, false, false, 0, 0),
                 Case("MIR-309.010", inc[2], box00, {0, box00.west(), 0, box00.west()}, false, true,
                      0, 0),
                 Case("MIR-309.011", inc[2], box00, {box00.south(), 0, box00.south(), 0}, true,
                      false, 0, 0),
                 Case("MIR-309.012", inc[2], box00, box00, true, true, 0, 0),

                 Case("MIR-309.013", inc[0], box11,
                      {box11.south(), box11.west(), box11.south(), box11.west()}, false, false, 0,
                      0),
                 Case("MIR-309.014", inc[0], box11,
                      {box11.south(), box11.west(), box11.south(), box11.west()}, false, true, 0,
                      0),
                 Case("MIR-309.015", inc[0], box11,
                      {box11.south(), box11.west(), box11.south(), box11.west()}, true, false, 0,
                      0),
                 Case("MIR-309.016", inc[0], box11,
                      {box11.south(), box11.west(), box11.south(), box11.west()}, true, true, 0, 0),

                 Case("MIR-309.017", inc[1], box11, {0, 0, 0, 0}, false, false, 0, 0),
                 Case("MIR-309.018", inc[1], box11, {0, box11.west(), 0, box11.east()}, false, true,
                      0, 0),
                 Case("MIR-309.019", inc[1], box11, {box11.north(), 0, box11.south(), 0}, true,
                      false, 0, 0),
                 Case("MIR-309.020", inc[1], box11,
                      {box11.north(), box11.west(), box11.south(), box11.east()}, true, true, 0, 0),

                 Case("MIR-309.021", inc[2], box11, {0, 0, 0, 0}, false, false, 0, 0),
                 Case("MIR-309.022", inc[2], box11, {0, box11.west(), 0, box11.west()}, false, true,
                      0, 0),
                 Case("MIR-309.023", inc[2], box11, {box11.south(), 0, box11.south(), 0}, true,
                      false, 0, 0),
                 Case("MIR-309.024", inc[2], box11,
                      {box11.south(), box11.west(), box11.south(), box11.west()}, true, true, 0, 0),

                 Case("MIR-309.025", inc[3], box11, {0, 0, 0, 0}, false, false, 0, 0),
                 Case("MIR-309.026", inc[3], box11, {0, box11.west(), 0, box11.west()}, false, true,
                      0, 0),
                 Case("MIR-309.027", inc[3], box11, {box11.south(), 0, box11.south(), 0}, true,
                      false, 0, 0),
                 Case("MIR-309.028", inc[3], box11,
                      {box11.south(), box11.west(), box11.south(), box11.west()}, true, true, 0, 0),

                 Case("MIR-309.029", inc[3], box22, {0, 0, 0, 0}, false, false, 0, 0),
                 Case("MIR-309.030", inc[3], box22, {0, box22.west(), 0, box22.west()}, false, true,
                      0, 0),
                 Case("MIR-309.031", inc[3], box22, {box22.south(), 0, box22.south(), 0}, true,
                      false, 0, 0),
                 Case("MIR-309.032", inc[3], box22,
                      {box22.south(), box22.west(), box22.south(), box22.west()}, true, true, 0, 0),

                 Case("MIR-313.001", Increments(0.45, 0.3),
                      BoundingBox(89.85, 0.225, -89.85, 359.775),
                      BoundingBox(89.85, 0.225, -89.85, 359.775), true, true, 0, 0),
                 Case("MIR-313.002", Increments(0.45, 0.3), BoundingBox(90, 0, -90, 359.55),
                      BoundingBox(90, 0, -90, 359.55), false, false, 0, 0),
                 Case("MIR-313.003", Increments(1.25, 0.833333), BoundingBox(90, 0, -90, 358.75),
                      BoundingBox(89.999928, 0, -90, 358.75), true, false, 0, 0),
                 Case("MIR-313.004", Increments(1.25, 0.833333),
                      BoundingBox(89.5833, 0.625, -89.5833, 359.375),
                      BoundingBox(89.583295, 0.625, -89.5833, 359.375), true, true, 0, 0),

                 Case("MIR-315.001", Increments{0.55469953775, 1.41732283465}, GLOBE, FIX, true,
                      true, 0, 0),
                 Case("MIR-315.002", Increments{0.555555555556, 1.41732283465}, GLOBE, FIX, true,
                      true, 0, 0),
                 Case("MIR-315.003", Increments{0.55832037325, 1.41732283465}, GLOBE, FIX, true,
                      true, 0, 0),
                 Case("MIR-315.004", Increments{0.559875583204, 1.41732283465}, GLOBE, FIX, true,
                      true, 0, 0),
                 Case("MIR-315.005", Increments{0.8333333333333334, 0.5555555555555556},
                      {90, -359.16666667, 29.44444468, 0.83333189}, FIX, true, true, 0, 0),
                 Case("MIR-315.006", Increments{0.04166666667, 0.04166666667},
                      {50.000004, 339.95836053, 19.95833493, 350.000028}, FIX, true, true, 0, 0),



             }) {
            log << "Test " << kase << ":" << std::endl;


            using eckit::types::is_approximately_equal;
            bool ok = true;

            PointLatLon ref(kase.boundingBox_.south(), kase.boundingBox_.west());

            static size_t c = 1;
            log << "Test " << c++ << ":"
                << "\n\t  name='" << kase.name_ << "'"
                << "\n\t  " << kase.increments_ << "\n\t  " << kase.boundingBox_ << "\n\t   reference=" << ref
                << "should result in"
                << "\n\t  " << kase.correctBoundingBox_ << "\n\t  allow latitude shift? "
                << std::boolalpha << kase.allowLatitudeShift_ << std::noboolalpha
                << "\n\t  allow longitude shift? " << std::boolalpha << kase.allowLongitudeShift_
                << std::noboolalpha << std::endl;

            repres::latlon::RegularLL ll(kase.increments_, kase.boundingBox_, ref);
            const BoundingBox& corrected = static_cast<repres::Representation&>(ll).boundingBox();

            if (!is_approximately_equal(kase.boundingBox_.south().value(), ref.lat().value()) ||
                !is_approximately_equal(kase.boundingBox_.west().value(), ref.lon().value())) {
                ok = false;
                log << "\nPreserving the reference point " << ref << " failed:"
                    << "\n\t   " << kase
                    << std::endl;
            }

            if (!kase.boundingBox_.contains(corrected)) {
                ok = false;
                log << "\n" << kase.boundingBox_ << " does not contain " << ll << std::endl;
            }

            if (ll.Ni() != kase.ni_ || ll.Nj() != kase.nj_) {
                ok = false;
                log << "\nCalculation of (Ni, Nj) = (" << kase.ni_ << ", " << kase.nj_ << ") failed:"
                    << "\n\t   (" << ll.Ni() << ", " << ll.Nj() << ")" << std::endl;
            }


            EXPECT(kase.boundingBox_.empty() || kase.boundingBox_.contains(corrected));
            EXPECT(kase.correctBoundingBox_ == corrected);

            if (kase.boundingBox_.north() == kase.boundingBox_.south()) {
                EXPECT(corrected.north() == corrected.south());
            }
            else {
                EXPECT(kase.allowLatitudeShift_ || !kase.increments_.isLatitudeShifted(corrected));
            }

            if (kase.boundingBox_.east() == kase.boundingBox_.west()) {
                EXPECT(corrected.east() == corrected.west());
            }
            else {
                EXPECT(kase.allowLongitudeShift_ || !kase.increments_.isLongitudeShifted(corrected));
            }


            log << std::boolalpha << "Testing: " << ok << std::noboolalpha << std::endl;
            EXPECT(ok);

        }
    }


    SECTION("LatLon::globaliseBoundingBox()):") {

        // the second Case represents the globe
        using test_t = std::pair<Case, Case>;

        for (const test_t& kase :
             {test_t{Case("001.area", inc[1], {0, -350, 0, 8}, FIX, true, true, 359, 1),
                     Case("001.global", inc[1], {90, 0, -90, 359}, FIX, true, true, 360, 181)},

              test_t{Case("002.area", Increments(0.5, 0.5), {0, -350, 0, 9}, FIX, true, true, 719, 1),
                     Case("002.global", Increments(0.5, 0.5), {90, 0, -90, 359.5}, FIX, true, true, 720, 361)},

              test_t{Case("003.area", inc[2], {2, 0, 0, 2}, FIX, true, true, 2, 2),
                     Case("003.global", inc[2], {90, 0, -90, 358}, FIX, true, true, 180, 91)},

              test_t{Case("004.area", inc[2], {2.1, 0, 0, 2.1}, FIX, true, true, 2, 2),
                     Case("004.global", inc[2], {90, 0, -90, 358}, FIX, true, true, 180, 91)},

              test_t{Case("005.area", inc[2], {3, 1, 1, 3}, FIX, true, true, 2, 2),
                     Case("005.global", inc[2], {89, 1, -89, 359}, FIX, true, true, 180, 90)},

              test_t{Case("006.area", inc[2], {37.6025, -114.8907, 27.7626, -105.1875}, FIX, true, true, 5, 5),
                     Case("006.global", inc[2], {89.7626, 1.1093, -88.2374, 359.1093}, FIX, true, true, 180, 90)},

              test_t{Case("007.area", inc[2], {37.6025, -114.8907, 27.7626, -105.188}, FIX, true, true, 5, 5),
                     Case("007.global", inc[2], {89.7626, 1.1093, -88.2374, 359.1093}, FIX, true, true, 180, 90)},

              test_t{Case("008.area", inc[2], {88, -178, -88, 180}, FIX, true, true, 180, 89),
                     Case("008.global", inc[2], {90, 0, -90, 358}, FIX, true, true, 180, 91)},

              test_t{Case("009.area", inc[7], {85, 0, -90, 357}, FIX, true, true, 52, 26),
                     Case("009.global", inc[7], {85, 0, -90, 357}, FIX, true, true, 52, 26)},

              test_t{Case("010.area", inc[7], GLOBE, FIX, true, true, 52, 26),
                     Case("010.global", inc[7], {85, 0, -90, 357}, FIX, true, true, 52, 26)},

              test_t{Case("011.area", inc[4], {90, 0, -90, 356}, FIX, true, true, 90, 46),
                     Case("011.global", inc[4], {90, 0, -90, 356}, FIX, true, true, 90, 46)},

              test_t{Case("012.area", inc[4], GLOBE, FIX, true, true, 90, 46),
                     Case("012.global", inc[4], {90, 0, -90, 356}, FIX, true, true, 90, 46)

              }}) {
            ASSERT(kase.first.increments_ == kase.second.increments_);
            log << "Test increments=" << kase.first.increments_ << " with cases=["
                << "\n\t" << kase.first << ","
                << "\n\t" << kase.second << "]:"
                << std::endl;

            using repres::latlon::RegularLL;

            // check if Ni/Nj are well calculated, for the user-provided area
            PointLatLon ref(kase.first.boundingBox_.south(), kase.first.boundingBox_.west());
            repres::RepresentationHandle user =
                new RegularLL(kase.first.increments_, kase.first.boundingBox_, ref);

            size_t Ni = dynamic_cast<const RegularLL&>(*user).Ni();
            size_t Nj = dynamic_cast<const RegularLL&>(*user).Nj();

            EXPECT(kase.first.compare(Case(kase.first.name_ + ".calculated", kase.first.increments_, kase.first.boundingBox_, FIX, true, true, Ni, Nj)));

            // check if Ni/Nj are well calculated, for the 'globalised' area
            BoundingBox global(kase.first.boundingBox_);
            repres::latlon::LatLon::globaliseBoundingBox(global, kase.first.increments_, ref);

            repres::RepresentationHandle globalised = new RegularLL(kase.first.increments_, global, ref);

            Ni = dynamic_cast<const RegularLL&>(*globalised).Ni();
            Nj = dynamic_cast<const RegularLL&>(*globalised).Nj();

            EXPECT(kase.second.compare(
                Case(kase.first.name_ + ".globalised", kase.first.increments_, global, FIX, true, true, Ni, Nj) ));
        }
    }

   log << std::noboolalpha;
    log.precision(old);
}
#endif


}  // namespace unit
}  // namespace tests
}  // namespace mir


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
