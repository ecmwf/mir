/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @date Dec 2016


#include "mir/compare/Same.h"

//include <algorithm>
//include <cmath>
#include "mir/util/Angles.h"
//include "mir/config/LibMir.h"
//include "mir/data/MIRField.h"
//include "mir/param/MIRParametrisation.h"
//include "mir/repres/Representation.h"
//include "mir/util/Compare.h"
#include "mir/compare/Field.h"



namespace mir {
namespace compare {


Same::Same(const param::MIRParametrisation& param1, const param::MIRParametrisation& param2) :
    Comparator(param1, param2) {
    absoluteError_ = getSameParameter<double>("absolute-error");
    ASSERT(absoluteError_ > 0);
}


Same::~Same() {
}


bool Same::sameAccuracy(const Field& a, const Field& b) const {
    return true;
    if (a.accuracy_ == 0 || b.accuracy_ == 0) {
        return true;
    }
    return a.accuracy_  == b.accuracy_;
}


bool Same::sameArea(const Field& a, const Field& b) const {
    if (!a.area_ && !b.area_)
        return true;

    if (a.area_ != b.area_)
        return false;

    return compareAreas(a, b) > 0.7; // Observed for N320
}


bool Same::sameBitmap(const Field& a, const Field& b) const {
    return true;
}


bool Same::sameField(const Field& a, const Field& b) const {
    return a.values_ == b.values_;
}


bool Same::sameFormat(const Field& a, const Field& b) const {
    return a.format_ == b.format_;
}


bool Same::sameGridname(const Field& a, const Field& b) const {
    return a.gridname_ == b.gridname_;
}


bool Same::sameGrid(const Field& a, const Field& b) const {
    if (a.grid_ != b.grid_) {
        return false;
    }

    if (a.grid_) {
        return (a.north_south_ == b.north_south_) &&
               (a.west_east_ == b.west_east_) ;
    }

    return true;
}


bool Same::sameGridtype(const Field& a, const Field& b) const {
    return a.gridtype_ == b.gridtype_;
}


bool Same::sameNumberOfPoints(const Field& a, const Field& b) const {
    return true;
    return a.numberOfPoints_  == b.numberOfPoints_;
}


bool Same::samePacking(const Field& a, const Field& b) const {
    return true;

    if (a.accuracy_ == 0 || b.accuracy_ == 0) {
        return true;
    }

    if (a.packing_ == "grid_simple_matrix" && b.packing_ == "grid_simple") {
        return true;
    }

    if (a.packing_ == "grid_simple" && b.packing_ == "grid_simple_matrix") {
        return true;
    }

    // if (a.packing_ == "grid_second_order" && b.packing_ == "grid_simple") {
    //     return true;
    // }

    // if (a.packing_ == "grid_simple" && b.packing_ == "grid_second_order") {
    //     return true;
    // }

    //  if (a.packing_ == "grid_jpeg" && b.packing_ == "grid_simple") {
    //     return true;
    // }

    // if (a.packing_ == "grid_simple" && b.packing_ == "grid_jpeg") {
    //     return true;
    // }

    return a.packing_ == b.packing_;
}


bool Same::sameParam(const Field& a, const Field& b) const {
    return a.param_ == b.param_;
}


bool Same::sameResol(const Field& a, const Field& b) const {
    return a.resol_ == b.resol_;
}


bool Same::sameRotation(const Field& a, const Field& b) const {

    if (a.rotation_ != b.rotation_) {
        return false;
    }

    if (a.rotation_) {

        return (a.rotation_latitude_ == b.rotation_latitude_) &&
               (util::angles::between_0_and_360(a.rotation_longitude_) ==
                util::angles::between_0_and_360(b.rotation_longitude_));
    }

    return true;
}


double Same::compareAreas(const Field& a, const Field& b) const {

    if (!a.area_ || !b.area_) {
        return -1;
    }

    double w1 = a.west_;
    double e1 = a.east_;
    double n1 = a.north_;
    double s1 = a.south_;

    double w2 = b.west_;
    double e2 = b.east_;
    double n2 = b.north_;
    double s2 = b.south_;

    if (s1 <= -78) {
        auto j = a.values_.find("param");
        if (j != a.values_.end()) {
            auto s = (*j).second;
            if (s.length() == 6 && s.substr(0, 3) == "140") {
                s1 = -90;
            }
        }
    }

    if (s2 <= -78) {
        auto j = b.values_.find("param");
        if (j != b.values_.end()) {
            auto s = (*j).second;
            if (s.length() == 6 && s.substr(0, 3) == "140") {
                s2 = -90;
            }
        }
    }



    while (w1 >= e1) {
        w1 -= 360;
    }

    while (w2 >= e2) {
        w2 -= 360;
    }

    while (w1 < 0) {
        w1 += 360;
        e1 += 360;
    }

    while (w2 < 0) {
        w2 += 360;
        e2 += 360;
    }

    double ww = std::min(w1, w2);

    while (w1 >= ww + 360) {
        w1 -= 360;
        e1 -= 360;
    }

    while (w2 >= ww + 360) {
        w2 -= 360;
        e2 -= 360;
    }

//==========

    // Union
    double un = std::max(n1, n2);
    double us = std::min(s1, s2);
    double ue = std::max(e1, e2);
    double uw = std::min(w1, w2);

    double uarea = (un - us) * (ue - uw);


    if ((un - us) < 0 || (ue - uw) < 0) {
        std::ostringstream oss;
        oss << "Cannot compute union: "
            << n1 << "/" << w1 << "/" << s1 << "/" << e1 << " and "
            << n2 << "/" << w2 << "/" << s2 << "/" << e2 << " ==> "
            << un << "/" << uw << "/" << us << "/" << ue;
        oss << std::endl << *this;
        oss << std::endl << b;
        throw eckit::SeriousBug(oss.str());

    }


    // Intersction
    double in = std::min(n1, n2);
    double is = std::max(s1, s2);
    double ie = std::min(e1, e2);
    double iw = std::max(w1, w2);

    double iarea = (in - is) * (ie - iw);

    if (uarea == 0) {


        if (un == us) {
            ASSERT(ue > uw);

            if (in != is) {
                return 0;
            }

            uarea = ue - uw;
            iarea = ie - iw;
        }

        if (ue == uw) {
            ASSERT(un != us);

            if (ie != iw) {
                return 0;
            }

            uarea = un - us;
            iarea = in - is;
        }

        ASSERT(uarea > 0);

    }

    if ((in - is) < 0 || (ie - iw) < 0) {
        iarea = 0;
    }

    return iarea / uarea;

}


void Same::execute(const data::MIRField& a, const data::MIRField& b) const {
#if 0
    bool result = sameParam       (a, b)
            && sameField          (a, b)
            && sameNumberOfPoints (a, b)
            && sameGrid           (a, b)
            && sameAccuracy       (a, b)
            && samePacking        (a, b)
            && sameRotation       (a, b)
            && sameResol          (a, b)
            && sameGridname       (a, b)
            && sameGridtype       (a, b)
            && sameFormat         (a, b)
            && sameArea           (a, b);
#endif
}


void Same::print(std::ostream& out) const {
    out << "Same["
        << "absoluteError=" << absoluteError_
        << "]";
}


namespace {
ComparatorBuilder<Same> __sameComparator("XXX");
}


}  // namespace compare
}  // namespace mir


