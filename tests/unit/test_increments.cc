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
#include "mir/util/BoundingBox.h"
#include "mir/util/Increments.h"


using namespace eckit::testing;


namespace mir {
namespace tests {
namespace unit {


using util::BoundingBox;
using util::Increments;


struct LatLon {

    static bool includesNorthPole(const Increments& increments, const BoundingBox& bbox) {

        // if latitude range spans the globe, or within one increment from bounding box North
        const Latitude range = bbox.north() - bbox.south();
        const Latitude reach = std::min(bbox.north() + increments.south_north().latitude(), Latitude::NORTH_POLE);

        return  range.sameWithGrib1Accuracy(Latitude::GLOBE) ||
                reach.sameWithGrib1Accuracy(Latitude::NORTH_POLE);
    }

    static bool includesSouthPole(const Increments& increments, const BoundingBox& bbox) {

        // if latitude range spans the globe, or within one increment from bounding box South
        const Latitude range = bbox.north() - bbox.south();
        const Latitude reach = std::max(bbox.south() - increments.south_north().latitude(), Latitude::SOUTH_POLE);

        return  range.sameWithGrib1Accuracy(Latitude::GLOBE) ||
                reach.sameWithGrib1Accuracy(Latitude::SOUTH_POLE);
    }

    static bool isPeriodicWestEast(const Increments& increments, const BoundingBox& bbox) {
        const Longitude we = bbox.east() - bbox.west();
        const Longitude inc = increments.west_east().longitude();

        return  (we + inc).sameWithGrib1Accuracy(Longitude::GLOBE) ||
                (we + inc) > Longitude::GLOBE;
    }
};


struct Case {
    Case(const std::string& name,
         const BoundingBox& boundingBox,
         size_t Ni,
         size_t Nj,
         bool isLatitudeShifted,
         bool isLongitudeShifted) :
        name_(name),
        boundingBox_(boundingBox),
        Ni_(Ni),
        Nj_(Nj),
        isLatitudeShifted_(isLatitudeShifted),
        isLongitudeShifted_(isLongitudeShifted) {}

    void print(std::ostream& out) const {
        out << "name='" << name_ << "',\t"
               "Case["
               ",boundingBox="           << boundingBox_
            << ",Ni="                    << Ni_
            << ",Nj="                    << Nj_
            << ",isLatitudeShifted_?"    << isLatitudeShifted_
            << ",isLongitudeShifted_?"   << isLongitudeShifted_
            << "]";
    }

    const std::string name_;
    BoundingBox boundingBox_;
    size_t Ni_;
    size_t Nj_;
    bool isLatitudeShifted_;
    bool isLongitudeShifted_;

    bool compare(const Case& other) const {
        if (boundingBox_ != other.boundingBox_ ||
            Ni_ != other.Ni_ ||
            Nj_ != other.Nj_ ||
            isLatitudeShifted_  != other.isLatitudeShifted_ ||
            isLongitudeShifted_ != other.isLongitudeShifted_) {
            eckit::Log::debug<LibMir>() << "\n\t" "   " << *this
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
                          bool allowLatitudeShift,
                          bool allowLongitudeShift,
                          const Case& user,
                          const Case& globalised) :
        increments_(increments),
        allowLatitudeShift_(allowLatitudeShift),
        allowLongitudeShift_(allowLongitudeShift),
        user_(user),
        globalised_(globalised) {}

    void print(std::ostream& out) const {
        out << "UserAndGlobalisedCase["
               "\n\t" "allowLatitudeShift?"    << allowLatitudeShift_ << ","
               "\n\t" "allowLongitudeShift?"   << allowLongitudeShift_ << ","
               "\n\t" "user=" << user_ << ","
               "\n\t" "globalised=" << globalised_
            << "]";
    }

    const Increments& increments() const {
        return increments_;
    }

    const Increments increments_;
    const bool allowLatitudeShift_;
    const bool allowLongitudeShift_;
    const Case user_;
    const Case globalised_;

    bool check() const {

        // check if Ni/Nj and shifts are well calculated, for the user-provided area
        if (!user_.compare(Case("calculated",
                                user_.boundingBox_,
                                increments_.computeNi(user_.boundingBox_),
                                increments_.computeNj(user_.boundingBox_),
                                increments_.isLatitudeShifted(user_.boundingBox_),
                                increments_.isLongitudeShifted(user_.boundingBox_) ))) {
            return false;
        }

        // 'globalise' user-provided area, check if allowed shifts are respected
        BoundingBox global(user_.boundingBox_);
        increments_.globaliseBoundingBox(global, allowLongitudeShift_, allowLatitudeShift_);

        const bool isLatitudeShifted  = increments_.isLatitudeShifted(global);
        const bool isLongitudeShifted = increments_.isLongitudeShifted(global);
        if ((!allowLatitudeShift_  && isLatitudeShifted) ||
            (!allowLongitudeShift_ && isLongitudeShifted)) {
            eckit::Log::debug<LibMir>() << "globaliseBoundingBox(" << user_ << ") = " << global << ":"
                                        << "\t" "allowLatitudeShift_?  " << allowLatitudeShift_  << " isLatitudeShifted?  " << isLatitudeShifted
                                        << "\t" "allowLongitudeShift_? " << allowLongitudeShift_ << " isLongitudeShifted? " << isLongitudeShifted
                                        << std::endl;
            return false;
        }

        // check if Ni/Nj and shifts are well calculated, for the 'globalised' area
        if (!globalised_.compare(Case("calculated",
                                      global,
                                      increments_.computeNi(global),
                                      increments_.computeNj(global),
                                      isLatitudeShifted,
                                      isLongitudeShifted))) {

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

            BoundingBox maybe(n, w, s, e);
            std::streamsize p = eckit::Log::debug<LibMir>().precision(15);
            eckit::Log::debug<LibMir>() << "globaliseBoundingBox should maybe result in (CONFIRM FIRST!):"
                                        << "\n\t" << maybe
                                        << "\n\t" "Ni=" << increments_.computeNi(maybe)
                                        << "\n\t" "Nj=" << increments_.computeNj(maybe)
                                        << "\n\t" "includesNorthPole?  " << LatLon::includesNorthPole(increments_, maybe)
                                        << "\n\t" "includesSouthPole?  " << LatLon::includesSouthPole(increments_, maybe)
                                        << "\n\t" "isPeriodicWestEast? " << LatLon::isPeriodicWestEast(increments_, maybe)
                                        << std::endl;
            eckit::Log::debug<LibMir>().precision(p);

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
    for (const auto& cases : std::vector< UserAndGlobalisedCase >({

        { Increments(2, 2), true, true,                                     // increments, allowed lat/lon shift
          Case("user",   {   2,   0,   0,   2   },   2,  2, false, false),  // user bbox, Ni, Nj, is lat/lon shifted
          Case("global", {  90,   0, -90, 358   }, 180, 91, false, false)   // globalised bbox ...
        },

        { Increments(2, 2), true, true,
          Case("user",   {   2.1, 0,   0,   2.1 },   2,  2, false, false),
          Case("global", {  90,   0, -90, 358   }, 180, 91, false, false)
        },

        { Increments(2, 2), true, true,
          Case("user",   {  3, 1,   1,   3 },   2,  2, true, true),
          Case("global", { 89, 1, -89, 359 }, 180, 90, true, true),
        },

#if 0
        { Increments(2, 2), true, true,
          Case("user",   { 37.6025,           -114.8907,             27.7626, -105.1875}, 4, 4, true, true),
          Case("global", { 89.7626,           -114.8907,            -88.2374,  243.1093}, 1, 1, true, true),
        },
#endif

#if 0
        { Increments(2, 2), true, true,
          Case("user",   { 37.6025,           -114.8915,             27.7626, -105.188  },   4,  4, true, true),
          Case("global", { 89.7626,           -114.8915,            -88.2374,  243.1085 }, 179,  4, true, true),
        },
#endif

#if 0
        { Increments(2, 2), true, true,
          Case("user",   { 88,                -178,                 -88,       180},      1, 1, false, false),
          Case("global", { 88,                -178,                 -88,       180},      1, 1, false, false),
        },
#endif

        })) {
        eckit::Log::debug<LibMir>() << "Test increments=" << cases.increments() << " with cases=" << cases << ":" << std::endl;
        EXPECT( cases.check() );
    }
}


}  // namespace unit
}  // namespace tests
}  // namespace mir


int main(int argc, char **argv) {
    return run_tests(argc, argv, false);
}

