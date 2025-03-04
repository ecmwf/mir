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
#include <ios>
#include <memory>
#include <ostream>
#include <string>
#include <vector>

#include "eckit/testing/Test.h"
#include "eckit/types/FloatCompare.h"
#include "eckit/types/Fraction.h"

#include "mir/data/MIRField.h"
#include "mir/input/GribFileInput.h"
#include "mir/repres/latlon/RegularLL.h"
#include "mir/util/BoundingBox.h"
#include "mir/util/Domain.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Increments.h"
#include "mir/util/Log.h"
#include "mir/util/Types.h"


namespace mir::tests::unit {


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

        const auto& param = input->parametrisation();

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


CASE("Increments::correctBoundingBox") {

    auto old(log.precision(16));
    log << std::boolalpha;


    SECTION("basic") {
        auto test = [](bool shiftLon, bool shiftLat, size_t Ni, size_t Nj) {
            Increments inc{1, 1};
            BoundingBox box;
            PointLatLon ref{shiftLat ? 0.5 : 0., shiftLon ? 0.5 : 0.};

            repres::RepresentationHandle repres(new repres::latlon::RegularLL(inc, box, ref));
            const auto& ll = dynamic_cast<const repres::latlon::RegularLL&>(*repres);

            static size_t c = 1;
            log << "Test " << c++ << ":"
                << "\n\t"
                   "   RegularLL(increments="
                << inc << ", reference=" << ref << ")"
                << "\n\t"
                   " = "
                << ll << std::endl;

            EXPECT(ll.Ni() == Ni);
            EXPECT(ll.Nj() == Nj);

            const auto& bbox = repres->boundingBox();
            EXPECT(bbox.west() == ref.lon());
            EXPECT(bbox.east() == bbox.west() + double(ll.Ni() - 1));
            EXPECT(bbox.north() == bbox.south() + double(ll.Nj() - 1));

            repres->validate(MIRValuesVector(Ni * Nj));
        };

        test(false, false, 360, 181);
        test(true, false, 360, 181);
        test(false, true, 360, 180);
        test(true, true, 360, 180);
    }


    SECTION("small areas") {
        Increments inc(3, 3);

        for (auto& box : std::vector<BoundingBox>{
                 {4, -1, -1, 4},
                 {12, -1, -1, 12},
             }) {

            for (auto& reference : std::vector<PointLatLon>{
                     {0, 0},
                     {0, -1},
                     {-1, 0},
                     {1, -1},
                     {0, 1},
                     {1, 0},
                     {1, 1},
                 }) {

                auto sn                   = inc.south_north().latitude();
                auto we                   = inc.west_east().longitude();
                bool expectLatitudeShift  = !(reference.lat().fraction() / sn.fraction()).integer();
                bool expectLongitudeShift = !(reference.lon().fraction() / we.fraction()).integer();

                auto corrected(box);
                size_t ni;
                size_t nj;
                repres::latlon::LatLon::correctBoundingBox(corrected, ni, nj, inc, reference);

                static size_t c = 1;
                log << "Test " << c++ << ":"
                    << "\n\t   " << inc << "\n\t + " << box << "\n\t > " << corrected << " corrected with reference "
                    << reference << "\n\t > ni = " << ni << "\n\t > nj = " << nj << "\n\t = shifted in latitude? "
                    << inc.isLatitudeShifted(corrected) << "\n\t = shifted in longitude? "
                    << inc.isLongitudeShifted(corrected) << std::endl;

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

                EXPECT(expectLatitudeShift || !inc.isLatitudeShifted(corrected));
                EXPECT(expectLongitudeShift || !inc.isLongitudeShifted(corrected));
            }
        }
    }


    SECTION("equator") {
        BoundingBox equator(0, -1, 0, 359);

        for (const auto& inc : {
                 Increments{3, 3},
                 Increments{7, 0},
             }) {

            for (auto& reference : std::vector<PointLatLon>{
                     {0, 0},
                     {0, -1},
                     {-1, 0},
                     {1, -1},
                     {0, 1},
                     {1, 0},
                     {1, 1},
                 }) {

                auto sn = inc.south_north().latitude();
                auto we = inc.west_east().longitude();

                auto corrected = equator;
                size_t ni;
                size_t nj;
                repres::latlon::LatLon::correctBoundingBox(corrected, ni, nj, inc, reference);

                static size_t c = 1;
                log << "Test " << c++ << ":"
                    << "\n\t   " << inc << "\n\t + " << equator << "\n\t > " << corrected
                    << " corrected with reference " << reference << "\n\t > ni = " << ni << "\n\t > nj = " << nj
                    << "\n\t = shifted in latitude? " << inc.isLatitudeShifted(corrected)
                    << "\n\t = shifted in longitude? " << inc.isLongitudeShifted(corrected) << std::endl;

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
                }
                else {
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
    Case(const std::string&& name, const BoundingBox& boundingBox, size_t ni, size_t nj) :
        name_(name), boundingBox_(boundingBox), ni_(ni), nj_(nj) {}

    void print(std::ostream& out) const {
        out << "name='" << name_
            << "',\t"
               "Case["
               "boundingBox="
            << boundingBox_ << ",ni=" << ni_ << ",nj=" << nj_ << "]";
    }

    const std::string name_;
    BoundingBox boundingBox_;
    size_t ni_;
    size_t nj_;

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
        c.print(s);
        return s;
    }
};


struct UserAndGlobalisedCase {
    UserAndGlobalisedCase(const Increments& increments, const Case&& user, const Case&& globalised) :
        increments_(increments), user_(user), globalised_(globalised) {}

    void print(std::ostream& out) const {
        out << "UserAndGlobalisedCase["
               "\n\t"
            << user_
            << ","
               "\n\t"
            << globalised_ << "]";
    }

    const Increments& increments() const { return increments_; }

    const Increments increments_;
    const Case user_;
    const Case globalised_;

    bool check() const {
        using repres::latlon::RegularLL;

        // check if Ni/Nj and shifts are well calculated, for the user-provided area
        PointLatLon ref(user_.boundingBox_.south(), user_.boundingBox_.west());
        repres::RepresentationHandle user = new RegularLL(increments_, user_.boundingBox_, ref);
        const auto& user_ll               = dynamic_cast<const RegularLL&>(*user);

        if (!user_.compare(Case("calculated", user_.boundingBox_, user_ll.Ni(), user_ll.Nj()))) {
            return false;
        }

        // 'globalise' user-provided area, check if allowed shifts are respected
        BoundingBox global(user_.boundingBox_);
        repres::latlon::LatLon::globaliseBoundingBox(global, increments_, ref);

        repres::RepresentationHandle globalised = new RegularLL(increments_, global, ref);
        const auto& globalised_ll               = dynamic_cast<const RegularLL&>(*globalised);

        // check if Ni/Nj and shifts are well calculated, for the 'globalised' area
        if (!globalised_.compare(Case("calculated", global, globalised_ll.Ni(), globalised_ll.Nj()))) {

            Latitude s = user_.boundingBox_.south();
            Latitude n = s;
            const Latitude sn(increments_.south_north().latitude());

            while (n + sn <= Latitude::NORTH_POLE) {
                n += sn;
            }
            while (s - sn >= Latitude::SOUTH_POLE) {
                s -= sn;
            }

            Longitude w = user_.boundingBox_.west();
            Longitude e = w;
            const Longitude we(increments_.west_east().longitude());

            while (e - w >= Longitude::GLOBE) {
                e -= we;
            }
            while (e - w < Longitude::GLOBE - we) {
                e += we;
            }

            BoundingBox maybe_box(n, w, s, e);

            repres::RepresentationHandle maybe = new RegularLL(increments_, maybe_box, ref);
            const auto& maybe_ll               = dynamic_cast<const repres::latlon::RegularLL&>(*maybe);

            log << "globaliseBoundingBox should maybe result in (CONFIRM FIRST!):"
                << "\n\t" << maybe_box
                << "\n\t"
                   "Ni="
                << maybe_ll.Ni()
                << "\n\t"
                   "Nj="
                << maybe_ll.Nj()
                << "\n\t"
                   "includesPoleNorth?  "
                << maybe->domain().includesPoleNorth()
                << "\n\t"
                   "includesPoleSouth?  "
                << maybe->domain().includesPoleSouth()
                << "\n\t"
                   "isPeriodicWestEast? "
                << maybe->domain().isPeriodicWestEast() << std::endl;

            return false;
        }

        return true;
    }

    friend std::ostream& operator<<(std::ostream& s, const UserAndGlobalisedCase& c) {
        c.print(s);
        return s;
    }
};


CASE("test_increments") {

    auto old(log.precision(16));
    log << std::boolalpha;

    BoundingBox GLOBE;


    for (const auto& cases : std::vector<UserAndGlobalisedCase>({

             {
                 Increments(1, 1), Case("user", {0, -350, 0, 8}, 359, 1),  // user bbox, Ni, Nj
                 Case("global", {90, 0, -90, 359}, 360, 181)               // globalised bbox, Ni, Nj
             },

             {Increments(0.5, 0.5), Case("user", {0, -350, 0, 9}, 719, 1),
              Case("global", {90, 0, -90, 359.5}, 720, 361)},

             {Increments(2, 2), Case("user", {2, 0, 0, 2}, 2, 2), Case("global", {90, 0, -90, 358}, 180, 91)},

             {Increments(2, 2), Case("user", {2.1, 0, 0, 2.1}, 2, 2), Case("global", {90, 0, -90, 358}, 180, 91)},

             {
                 Increments(2, 2),
                 Case("user", {3, 1, 1, 3}, 2, 2),
                 Case("global", {89, 1, -89, 359}, 180, 90),
             },

             {
                 Increments(2, 2),
                 Case("user", {37.6025, -114.8907, 27.7626, -105.1875}, 5, 5),
                 Case("global", {89.7626, 1.1093, -88.2374, 359.1093}, 180, 90),
             },

             {
                 Increments(2, 2),
                 Case("user", {37.6025, -114.8907, 27.7626, -105.188}, 5, 5),
                 Case("global", {89.7626, 1.1093, -88.2374, 359.1093}, 180, 90),
             },

             {Increments(2, 2), Case("user", {88, -178, -88, 180}, 180, 89),
              Case("global", {90, 0, -90, 358}, 180, 91)},

             {Increments(7, 7), Case("user", {85, 0, -90, 357}, 52, 26), Case("global", {85, 0, -90, 357}, 52, 26)},

             {Increments(7, 7), Case("user", GLOBE, 52, 26), Case("global", {85, 0, -90, 357}, 52, 26)},

             {Increments(4, 4), Case("user", {90, 0, -90, 356}, 90, 46), Case("global", {90, 0, -90, 356}, 90, 46)},

             {Increments(4, 4), Case("user", GLOBE, 90, 46), Case("global", {90, 0, -90, 356}, 90, 46)},

         })) {
        log << "Test increments=" << cases.increments() << " with cases=" << cases << ":" << std::endl;
        EXPECT(cases.check());
    }


    log.precision(old);
}


CASE("RegularLL(Increments, BoundingBox)") {

    auto old(log.precision(16));
    log << std::boolalpha;

    struct test_t {
        Increments increments_;
        BoundingBox boundingBox_;
    };

    for (const auto& test : {

             // MIR-251
             test_t{Increments{0.003474259, 0.003496601747573},
                    BoundingBox{34.6548026, 113.04894614, 34.2911562, 113.7472724}},
             test_t{Increments{0.003474259, 0.003496601747573},
                    BoundingBox{34.6548026, 113.04894614, 34.2911562, 113.747272}},
             test_t{Increments{0.00352, 0.003558252427184}, BoundingBox{34.657355, 113.04832, 34.29085525, 113.74528}},
             test_t{Increments{0.006198529411765, 0.005665625},
                    BoundingBox{36.34501645, 113.58806225, 35.81244723, 114.41866527}},
             test_t{Increments{0.0097087, 0.010417}, BoundingBox{35.00112, 112.9995593, 33.990671, 114.0092641}},
             test_t{Increments{0.010417, 0.007353}, BoundingBox{36.345879, 113.586968, 35.816463, 114.420328}},
             test_t{Increments{0.166667, 0.166667}, BoundingBox{51.166769, 10.833355, 42.500085, 28.166723}},
             test_t{Increments{0.2999999, 0.3}, BoundingBox{-5.7, 105.8999647, -6, 106.1999646}},

             // MIR-315
             test_t{Increments{0.55469953775, 1.41732283465}, BoundingBox()},
             test_t{Increments{0.555555555556, 1.41732283465}, BoundingBox()},
             test_t{Increments{0.55832037325, 1.41732283465}, BoundingBox()},
             test_t{Increments{0.559875583204, 1.41732283465}, BoundingBox()},
             test_t{Increments{0.8333333333333334, 0.5555555555555556},
                    BoundingBox{90, -359.16666667, 29.44444468, 0.83333189}},
             test_t{Increments{0.04166666667, 0.04166666667},
                    BoundingBox{50.000004, 339.95836053, 19.95833493, 350.000028}},

         }) {
        static size_t c = 1;
        log << "Test " << c++ << ":"
            << "\n\t"
               "   "
            << test.increments_
            << "\n\t"
               " + "
            << test.boundingBox_ << std::endl;
        repres::RepresentationHandle repres(new repres::latlon::RegularLL(test.increments_, test.boundingBox_));
        log << "\t"
               " = "
            << *repres << std::endl;
    }

    log.precision(old);
}


CASE("MIR-309") {

    auto old(log.precision(16));
    log << std::boolalpha;

    struct Case {
        Case(const BoundingBox& bbox_, const Increments& increments_, bool allowLatitudeShift_,
             bool allowLongitudeShift_, const BoundingBox& correct_) :
            bbox(bbox_),
            increments(increments_),
            corrected(correct_),
            allowLatitudeShift(allowLatitudeShift_),
            allowLongitudeShift(allowLongitudeShift_) {}
        const BoundingBox bbox;
        const Increments increments;
        const BoundingBox corrected;
        const bool allowLatitudeShift;
        const bool allowLongitudeShift;
    };

    Increments inc00(0, 0);
    Increments inc11(1, 1);
    Increments inc22(2, 2);
    Increments inc33(3, 3);

    BoundingBox box00(-0.2, -0.3, -0.2, -0.3);
    BoundingBox box11(0.9, -0.2, -0.1, 0.8);
    BoundingBox box22(1, -1, -1, 1);
    BoundingBox box33(2, -1, -1, 2);


    SECTION("LatLon::correctBoundingBox") {
        for (Case& t : std::vector<Case>{

                 {box00, inc00, false, false, box00},
                 {box00, inc00, false, true, box00},
                 {box00, inc00, true, false, box00},
                 {box00, inc00, true, true, box00},

                 {box00, inc11, false, false, {0, 0, 0, 0}},
                 {box00, inc11, false, true, {0, box00.west(), 0, box00.west()}},
                 {box00, inc11, true, false, {box00.south(), 0, box00.south(), 0}},
                 {box00, inc11, true, true, box00},

                 {box00, inc22, false, false, {0, 0, 0, 0}},
                 {box00, inc22, false, true, {0, box00.west(), 0, box00.west()}},
                 {box00, inc22, true, false, {box00.south(), 0, box00.south(), 0}},
                 {box00, inc22, true, true, box00},

                 {box11, inc00, false, false, {box11.south(), box11.west(), box11.south(), box11.west()}},
                 {box11, inc00, false, true, {box11.south(), box11.west(), box11.south(), box11.west()}},
                 {box11, inc00, true, false, {box11.south(), box11.west(), box11.south(), box11.west()}},
                 {box11, inc00, true, true, {box11.south(), box11.west(), box11.south(), box11.west()}},

                 {box11, inc11, false, false, {0, 0, 0, 0}},
                 {box11, inc11, false, true, {0, box11.west(), 0, box11.east()}},
                 {box11, inc11, true, false, {box11.north(), 0, box11.south(), 0}},
                 {box11, inc11, true, true, {box11.north(), box11.west(), box11.south(), box11.east()}},

                 {box11, inc22, false, false, {0, 0, 0, 0}},
                 {box11, inc22, false, true, {0, box11.west(), 0, box11.west()}},
                 {box11, inc22, true, false, {box11.south(), 0, box11.south(), 0}},
                 {box11, inc22, true, true, {box11.south(), box11.west(), box11.south(), box11.west()}},

                 {box11, inc33, false, false, {0, 0, 0, 0}},
                 {box11, inc33, false, true, {0, box11.west(), 0, box11.west()}},
                 {box11, inc33, true, false, {box11.south(), 0, box11.south(), 0}},
                 {box11, inc33, true, true, {box11.south(), box11.west(), box11.south(), box11.west()}},

                 {box22, inc33, false, false, {0, 0, 0, 0}},
                 {box22, inc33, false, true, {0, box22.west(), 0, box22.west()}},
                 {box22, inc33, true, false, {box22.south(), 0, box22.south(), 0}},
                 {box22, inc33, true, true, {box22.south(), box22.west(), box22.south(), box22.west()}},

                 // MIR-313: UKMO shifted regular_ll (TIGGE)
                 {BoundingBox(89.85, 0.225, -89.85, 359.775), Increments(0.45, 0.3), true, true,
                  BoundingBox(89.85, 0.225, -89.85, 359.775)},

                 // MIR-313: UKMO non-shifted regular_ll (TIGGE)
                 {BoundingBox(90, 0, -90, 359.55), Increments(0.45, 0.3), false, false,
                  BoundingBox(90, 0, -90, 359.55)},

                 // MIR-313: UKMO shifted/non-shifted regular_ll (TIGGE)
                 {BoundingBox(90, 0, -90, 358.75), Increments(1.25, 0.833333), true, false,
                  BoundingBox(89.999928, 0, -90, 358.75)},

                 // MIR-313: UKMO shifted regular_ll (TIGGE)
                 {BoundingBox(89.5833, 0.625, -89.5833, 359.375), Increments(1.25, 0.833333), true, true,
                  BoundingBox(89.583295, 0.625, -89.5833, 359.375)},

             }) {

            PointLatLon ref(t.allowLatitudeShift ? t.bbox.south() : 0, t.allowLongitudeShift ? t.bbox.west() : 0);

            repres::RepresentationHandle rep(new repres::latlon::RegularLL(t.increments, t.bbox, ref));
            const BoundingBox& corrected = rep->boundingBox();

            static size_t c = 1;
            log << "Test " << c++ << ":"
                << "\n\t   " << t.bbox << "\n\t > " << corrected << "\n\t = " << t.corrected
                << "\n\t = shifted in latitude? " << t.increments.isLatitudeShifted(corrected)
                << (t.allowLatitudeShift ? "" : " (should be false)") << "\n\t = shifted in longitude? "
                << t.increments.isLongitudeShifted(corrected) << (t.allowLongitudeShift ? "" : " (should be false)")
                << std::endl;

            EXPECT(t.bbox.empty() || t.bbox.contains(corrected));
            EXPECT(t.corrected == corrected);

            if (t.bbox.north() == t.bbox.south()) {
                EXPECT(corrected.north() == corrected.south());
            }
            else {
                EXPECT(t.allowLatitudeShift || !t.increments.isLatitudeShifted(corrected));
            }

            if (t.bbox.east() == t.bbox.west()) {
                EXPECT(corrected.east() == corrected.west());
            }
            else {
                EXPECT(t.allowLongitudeShift || !t.increments.isLongitudeShifted(corrected));
            }
        }
    }


    log.precision(old);
}


}  // namespace mir::tests::unit


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
