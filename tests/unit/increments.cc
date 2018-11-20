/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include <iostream>
#include <string>
#include <vector>

#include "eckit/log/Log.h"
#include "eckit/memory/ScopedPtr.h"
#include "eckit/testing/Test.h"

#include "mir/config/LibMir.h"
#include "mir/repres/latlon/RegularLL.h"
#include "mir/util/BoundingBox.h"
#include "mir/util/Domain.h"
#include "mir/util/Increments.h"


namespace mir {
namespace tests {
namespace unit {


using util::BoundingBox;
using util::Increments;

static auto& log = eckit::Log::info();
using prec_t = decltype (log.precision());


CASE("Increments::correctBoundingBox") {

    auto old(log.precision(16));
    log << std::boolalpha;


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
                inc.correctBoundingBox(corrected, reference);

                auto Ni = 1 + (corrected.east() - corrected.west()).fraction() / we.fraction();
                auto Nj = 1 + (corrected.north() - corrected.south()).fraction() / sn.fraction();

                static size_t c = 1;
                log << "Test " << c++ << ":"
                    << "\n\t   " << inc
                    << "\n\t + " << box
                    << "\n\t > " << corrected << " corrected with reference " << reference
                    << "\n\t > Ni = " << Ni
                    << "\n\t > Nj = " << Nj
                    << "\n\t = shifted in latitude? " << inc.isLatitudeShifted(corrected)
                    << "\n\t = shifted in longitude? " << inc.isLongitudeShifted(corrected)
                    << std::endl;

                EXPECT(box.contains(corrected));
                EXPECT(Ni.integer() && Ni >= 1);
                EXPECT(Nj.integer() && Nj >= 1);

                EXPECT(corrected.west() == reference.lon());
                EXPECT(corrected.west() + (Ni - 1) * we.fraction() == corrected.east());
                EXPECT(corrected.west() + Ni * we.fraction() > box.east());

                EXPECT(corrected.south() == reference.lat());
                EXPECT(corrected.south() + (Nj - 1) * sn.fraction() == corrected.north());
                EXPECT(corrected.south() + Nj * sn.fraction() > box.north());

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
                inc.correctBoundingBox(corrected, reference);

                auto Ni = 1 + (corrected.east() - corrected.west()).fraction() / we.fraction();

                static size_t c = 1;
                log << "Test " << c++ << ":"
                    << "\n\t   " << inc
                    << "\n\t + " << equator
                    << "\n\t > " << corrected << " corrected with reference " << reference
                    << "\n\t > Ni = " << Ni
                    << "\n\t = shifted in latitude? " << inc.isLatitudeShifted(corrected)
                    << "\n\t = shifted in longitude? " << inc.isLongitudeShifted(corrected)
                    << std::endl;

                if (reference.lat() == 0) {
                    EXPECT(equator.contains(corrected));
                }
                EXPECT(Ni.integer() && Ni >= 1);

                if (sn == 0) {
                    EXPECT(corrected.south() == equator.south());
                }

                EXPECT(corrected.south() == corrected.north());
                EXPECT(corrected.west() == reference.lon());

                if (inc.isPeriodic()) {
                    EXPECT(Ni * we.fraction() == Longitude::GLOBE.fraction());
                } else {
                    auto Nabove = Ni;
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
    Case(const std::string&& name, const BoundingBox& boundingBox, size_t Ni, size_t Nj) :
        name_(name),
        boundingBox_(boundingBox),
        Ni_(Ni),
        Nj_(Nj) {}

    void print(std::ostream& out) const {
        out << "name='" << name_ << "',\t"
               "Case["
                "boundingBox="         << boundingBox_
            << ",Ni="                  << Ni_
            << ",Nj="                  << Nj_
            << "]";
    }

    const std::string name_;
    BoundingBox boundingBox_;
    size_t Ni_;
    size_t Nj_;

    bool compare(const Case& other) const {
        if (boundingBox_ != other.boundingBox_ || Ni_ != other.Ni_ || Nj_ != other.Nj_) {
            log << "\n\t" "   " << *this
                << "\n\t" "!= " << other
                << std::endl;
            return false;
        }
        return true;
    }

    friend std::ostream &operator<<(std::ostream& s, const Case& c) {
        c.print(s);
        return s;
    }
};


struct UserAndGlobalisedCase {
    UserAndGlobalisedCase(const Increments& increments,
                          const Case&& user,
                          const Case&& globalised) :
        increments_(increments),
        user_(user),
        globalised_(globalised) {}

    void print(std::ostream& out) const {
        out << "UserAndGlobalisedCase["
               "\n\t" << user_ << ","
               "\n\t" << globalised_
            << "]";
    }

    const Increments& increments() const {
        return increments_;
    }

    const Increments increments_;
    const Case user_;
    const Case globalised_;

    bool check() const {
        using repres::latlon::RegularLL;

        // check if Ni/Nj and shifts are well calculated, for the user-provided area
        PointLatLon ref(user_.boundingBox_.south(),
                        user_.boundingBox_.west());
        repres::RepresentationHandle user = new RegularLL(increments_, user_.boundingBox_, ref);
        auto& user_ll = dynamic_cast<const RegularLL&>(*user);

        if (!user_.compare(Case("calculated", user_.boundingBox_, user_ll.Ni(), user_ll.Nj() ))) {
            return false;
        }

        // 'globalise' user-provided area, check if allowed shifts are respected
        BoundingBox global(user_.boundingBox_);
        increments_.globaliseBoundingBox(global, ref);

        repres::RepresentationHandle globalised = new RegularLL(increments_, global, ref);
        auto& globalised_ll = dynamic_cast<const RegularLL&>(*globalised);

        // check if Ni/Nj and shifts are well calculated, for the 'globalised' area
        if (!globalised_.compare(Case("calculated", global, globalised_ll.Ni(), globalised_ll.Nj() ))) {

            Latitude s = user_.boundingBox_.south();
            Latitude n = s;
            const Latitude sn(increments_.south_north().latitude());

            while (n + sn <= Latitude::NORTH_POLE) { n += sn; }
            while (s - sn >= Latitude::SOUTH_POLE) { s -= sn; }

            Longitude w = user_.boundingBox_.west();
            Longitude e = w;
            const Longitude we(increments_.west_east().longitude());

            while (e - w >= Longitude::GLOBE)      { e -= we; }
            while (e - w <  Longitude::GLOBE - we) { e += we; }

            BoundingBox maybe_box(n, w, s, e);

            repres::RepresentationHandle maybe = new RegularLL(increments_, maybe_box, ref);
            auto& maybe_ll = dynamic_cast<const repres::latlon::RegularLL&>(*maybe);

            log << "globaliseBoundingBox should maybe result in (CONFIRM FIRST!):"
                << "\n\t" << maybe_box
                << "\n\t" "Ni=" << maybe_ll.Ni()
                << "\n\t" "Nj=" << maybe_ll.Nj()
                << "\n\t" "includesPoleNorth?  " << maybe->domain().includesPoleNorth()
                << "\n\t" "includesPoleSouth?  " << maybe->domain().includesPoleSouth()
                << "\n\t" "isPeriodicWestEast? " << maybe->domain().isPeriodicWestEast()
                << std::endl;

            return false;
        }

        return true;
    }

    friend std::ostream& operator<<(std::ostream& s, const UserAndGlobalisedCase& c) {
        c.print(s);
        return s;
    }
};


CASE( "test_increments" ) {

    auto old(log.precision(16));
    log << std::boolalpha;

    BoundingBox GLOBE;

    for (const auto& cases : std::vector< UserAndGlobalisedCase >({

        { Increments(1, 1),
          Case("user",   {   0, -350,   0,   8 }, 359,   1),  // user bbox, Ni, Nj
          Case("global", {  90,    0, -90, 359 }, 360, 181)   // globalised bbox, Ni, Nj
        },

        { Increments(0.5, 0.5),
          Case("user",   {   0, -350,   0,   9   }, 719,   1),
          Case("global", {  90,    0, -90, 359.5 }, 720, 361)
        },

        { Increments(2, 2),
          Case("user",   {   2,   0,   0,   2   },   2,  2),
          Case("global", {  90,   0, -90, 358   }, 180, 91)
        },

        { Increments(2, 2),
          Case("user",   {   2.1, 0,   0,   2.1 },   2,  2),
          Case("global", {  90,   0, -90, 358   }, 180, 91)
        },

        { Increments(2, 2),
          Case("user",   {  3, 1,   1,   3 },   2,  2),
          Case("global", { 89, 1, -89, 359 }, 180, 90),
        },

        { Increments(2, 2),
          Case("user",   { 37.6025, -114.8907,  27.7626, -105.1875 },   5,  5),
          Case("global", { 89.7626,    1.1093, -88.2374,  359.1093 }, 180, 90),
        },

        { Increments(2, 2),
          Case("user",   { 37.6025, -114.8907,  27.7626, -105.188 },   5,   5),
          Case("global", { 89.7626,    1.1093, -88.2374,  359.1093 }, 180, 90),
        },

        { Increments(2, 2),
          Case("user",   { 88, -178, -88, 180},    180,  89),
          Case("global", { 90,    0, -90, 358},    180,  91)
        },

        { Increments(7, 7),
          Case("user",   {  85,   0, -90, 357   },  52,  26),
          Case("global", {  85,   0, -90, 357   },  52,  26)
        },

        { Increments(7, 7),
          Case("user",   GLOBE,                     52,  26),
          Case("global", {  85,   0, -90, 357   },  52,  26)
        },

        { Increments(4, 4),
          Case("user",   {  90,   0, -90, 356   },  90,  46),
          Case("global", {  90,   0, -90, 356   },  90,  46)
        },

        { Increments(4, 4),
          Case("user",   GLOBE,                     90,  46),
          Case("global", {  90,   0, -90, 356   },  90,  46)
        },

        })) {
        log << "Test increments=" << cases.increments() << " with cases=" << cases << ":" << std::endl;
        EXPECT( cases.check() );
    }

    log.precision(old);
}


#if 0
CASE("MIR-251") {

    log.precision(16);
    log << std::boolalpha;

    struct test_t {
        Increments increments_;
        BoundingBox boundingBox_;
    };

    for (const auto& test : {
//         test_t{Increments{ 0.003474259,       0.003496601747573 }, BoundingBox{ 34.6548026,  113.04894614, 34.2911562,  113.747272 }},   // mir::repres::latlon::LatLon:44
         test_t{Increments{ 0.00352,           0.003558252427184 }, BoundingBox{ 34.657355,   113.04832,    34.29085525, 113.74528 }},    // mir::repres::latlon::LatLon:45
         test_t{Increments{ 0.006198529411765, 0.005665625 },       BoundingBox{ 36.34501645, 113.58806225, 35.81244723, 114.41866527 }}, // mir::repres::latlon::LatLon:45
         test_t{Increments{ 0.0097087,         0.010417 },          BoundingBox{ 35.00112,    112.9995593,  33.990671,   114.0092641 }},  // mir::repres::latlon::LatLon:45
         test_t{Increments{ 0.010417,          0.007353 },          BoundingBox{ 36.345879,   113.586968,   35.816463,   114.420328 }},   // mir::repres::latlon::LatLon:45
    }) {
        log << "Test:"
            << "\n\t" "   " << test.increments_
            << "\n\t" " + " << test.boundingBox_
            << std::endl;
        repres::RepresentationHandle repres(new repres::latlon::RegularLL(test.increments_, test.boundingBox_));
        log << "\t"   " = " << *repres
            << std::endl;
    }

    log.precision(old);
}
#endif


CASE("MIR-309") {

    auto old(log.precision(16));
    log << std::boolalpha;

    struct Case {
        Case(const BoundingBox& bbox_,
             const Increments& increments_,
             bool allowLatitudeShift_,
             bool allowLongitudeShift_,
             const BoundingBox& correct_) :
            bbox(bbox_),
            increments(increments_),
            corrected(correct_),
            allowLatitudeShift(allowLatitudeShift_),
            allowLongitudeShift(allowLongitudeShift_) {}
        const BoundingBox& bbox;
        const Increments& increments;
        const BoundingBox corrected;
        const bool allowLatitudeShift;
        const bool allowLongitudeShift;
    };

    Increments inc00(0, 0);
    Increments inc11(1, 1);
    Increments inc22(2, 2);
    Increments inc33(3, 3);

    BoundingBox box00(-0.2, -0.3, -0.2, -0.3);
    BoundingBox box11( 0.9, -0.2, -0.1,  0.8);
    BoundingBox box22(1, -1, -1, 1);
    BoundingBox box33(2, -1, -1, 2);

    SECTION("LatLon::correctBoundingBox") {
        for (Case& t : std::vector<Case>{

                { box00, inc00, false, false, box00 },
                { box00, inc00, false, true , box00 },
                { box00, inc00, true,  false, box00 },
                { box00, inc00, true,  true , box00 },

                { box00, inc11, false, false, {  0,            0,            0,             0            } },
                { box00, inc11, false, true , {  0,            box00.west(), 0,             box00.west() } },
                { box00, inc11, true,  false, { box00.south(), 0,            box00.south(), 0            } },
                { box00, inc11, true,  true , box00 },

                { box00, inc22, false, false, {  0,            0,            0,             0            } },
                { box00, inc22, false, true , {  0,            box00.west(), 0,             box00.west() } },
                { box00, inc22, true,  false, { box00.south(), 0,            box00.south(), 0            } },
                { box00, inc22, true,  true , box00 },

                { box11, inc00, false, false, { box11.south(), box11.west(), box11.south(), box11.west() } },
                { box11, inc00, false, true , { box11.south(), box11.west(), box11.south(), box11.west() } },
                { box11, inc00, true,  false, { box11.south(), box11.west(), box11.south(), box11.west() } },
                { box11, inc00, true,  true , { box11.south(), box11.west(), box11.south(), box11.west() } },

                { box11, inc11, false, false, {  0,            0,            0,             0            } },
                { box11, inc11, false, true , {  0,            box11.west(), 0,             box11.east() } },
                { box11, inc11, true,  false, { box11.north(), 0,            box11.south(), 0            } },
                { box11, inc11, true,  true , { box11.north(), box11.west(), box11.south(), box11.east() } },

                { box11, inc22, false, false, {  0,            0,            0,             0            } },
                { box11, inc22, false, true , {  0,            box11.west(), 0,             box11.west() } },
                { box11, inc22, true,  false, { box11.south(), 0,            box11.south(), 0            } },
                { box11, inc22, true,  true , { box11.south(), box11.west(), box11.south(), box11.west() } },

                { box11, inc33, false, false, {  0,            0,            0,             0            } },
                { box11, inc33, false, true , {  0,            box11.west(), 0,             box11.west() } },
                { box11, inc33, true,  false, { box11.south(), 0,            box11.south(), 0            } },
                { box11, inc33, true,  true , { box11.south(), box11.west(), box11.south(), box11.west() } },

                { box22, inc33, false, false, {  0,            0,            0,             0            } },
                { box22, inc33, false, true , {  0,            box22.west(), 0,             box22.west() } },
                { box22, inc33, true,  false, { box22.south(), 0,            box22.south(), 0            } },
                { box22, inc33, true,  true , { box22.south(), box22.west(), box22.south(), box22.west() } },

                // MIR-313: UKMO shifted regular_ll (TIGGE)
                { BoundingBox(89.85, 0.225, -89.85, 359.775),
                  Increments(0.45, 0.3),
                  true,
                  true,
                  BoundingBox(89.85, 0.225, -89.85, 359.775)
                },

                // MIR-313: UKMO non-shifted regular_ll (TIGGE)
                { BoundingBox(90, 0, -90, 359.55),
                  Increments(0.45, 0.3),
                  false,
                  false,
                  BoundingBox(90, 0, -90, 359.55)
                },

                // MIR-313: UKMO shifted/non-shifted regular_ll (TIGGE)
                { BoundingBox(90, 0, -90, 358.75),
                  Increments(1.25, 0.833333),
                  true,
                  false,
                  BoundingBox(89.999928, 0, -90, 358.75)
                },

                // MIR-313: UKMO shifted regular_ll (TIGGE)
                { BoundingBox(89.5833, 0.625, -89.5833, 359.375),
                  Increments(1.25, 0.833333),
                  true,
                  true,
                  BoundingBox(89.583295, 0.625, -89.5833, 359.375)
                },

            }) {

            PointLatLon ref(t.allowLatitudeShift? t.bbox.south() : 0.,
                            t.allowLongitudeShift? t.bbox.west() : 0);

            repres::RepresentationHandle rep(new repres::latlon::RegularLL(t.increments, t.bbox, ref));
            const BoundingBox& corrected = rep->boundingBox();

            static size_t c = 1;
            log << "Test " << c++ << ":"
                << "\n\t   " << t.bbox
                << "\n\t > " << corrected
                << "\n\t = " << t.corrected
                << "\n\t = shifted in latitude? " << t.increments.isLatitudeShifted(corrected) << (t.allowLatitudeShift? "" : " (should be false)")
                << "\n\t = shifted in longitude? " << t.increments.isLongitudeShifted(corrected) << (t.allowLongitudeShift? "" : " (should be false)")
                << std::endl;

            if (!t.bbox.empty()) {
                EXPECT(t.bbox.contains(corrected));
            }
            EXPECT(t.corrected == corrected);
            EXPECT(t.allowLatitudeShift || !t.increments.isLatitudeShifted(corrected));
            EXPECT(t.allowLongitudeShift || !t.increments.isLongitudeShifted(corrected));
        }
    }

    log.precision(old);
}


}  // namespace unit
}  // namespace tests
}  // namespace mir


int main(int argc, char **argv) {
    return eckit::testing::run_tests(argc, argv, false);
}

