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


#include "mir/util/Increments.h"

#include <ostream>

#include "eckit/log/JSON.h"
#include "eckit/types/FloatCompare.h"
#include "eckit/types/Fraction.h"

#include "mir/api/MIRJob.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/util/BoundingBox.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Grib.h"


namespace mir::util {


static void check(const Increments& inc) {
    ASSERT(inc.west_east() >= 0);
    ASSERT(inc.south_north() >= 0);
}


Increments::Increments(const param::MIRParametrisation& parametrisation) {
    double south_north;
    ASSERT(parametrisation.get("south_north_increment", south_north));
    south_north_ = south_north;

    double west_east;
    ASSERT(parametrisation.get("west_east_increment", west_east));
    west_east_ = west_east;

    check(*this);
}


Increments::Increments(double west_east, double south_north) : west_east_(west_east), south_north_(south_north) {
    check(*this);
}


bool Increments::operator==(const Increments& other) const {
    return eckit::types::is_approximately_equal(west_east_, other.west_east_) &&
           eckit::types::is_approximately_equal(south_north_, other.south_north_);
}


bool Increments::operator!=(const Increments& other) const {
    return !operator==(other);
}


bool Increments::isPeriodic() const {
    return (eckit::Fraction(360, 1) / eckit::Fraction(west_east_)).integer();
}


bool Increments::isShifted(const BoundingBox& bbox) const {
    const PointLonLat p{bbox.south(), bbox.west()};
    return isLatitudeShifted(p) || isLongitudeShifted(p);
}


bool Increments::isLatitudeShifted(const BoundingBox& bbox) const {
    return isLatitudeShifted(PointLonLat{bbox.south(), bbox.west()});
}


bool Increments::isLongitudeShifted(const BoundingBox& bbox) const {
    return isLongitudeShifted(PointLonLat{bbox.south(), bbox.west()});
}


bool Increments::isShifted(const PointLonLat& p) const {
    return isLatitudeShifted(p) || isLongitudeShifted(p);
}


bool Increments::isLatitudeShifted(const PointLonLat& p) const {
    const eckit::Fraction inc(south_north_);
    if (inc == 0) {
        return false;
    }
    return !(eckit::Fraction(p.lat) / inc).integer();
}


bool Increments::isLongitudeShifted(const PointLonLat& p) const {
    const eckit::Fraction inc(west_east_);
    if (inc == 0) {
        return false;
    }
    return !(eckit::Fraction(p.lon) / inc).integer();
}


void Increments::print(std::ostream& out) const {
    out << "Increments["
        << "west_east=" << west_east_ << ",south_north=" << south_north_ << "]";
}

void Increments::json(eckit::JSON& j) const {
    j.startObject();
    j << "west_east" << west_east_;
    j << "south_north" << south_north_;
    j.endObject();
}


void Increments::fillGrib(grib_info& info) const {
    // Warning: scanning mode not considered
    info.grid.iDirectionIncrementInDegrees = west_east_;
    info.grid.jDirectionIncrementInDegrees = south_north_;
}


void Increments::fillJob(api::MIRJob& job) const {
    job.set("grid", west_east_, south_north_);
}


void Increments::makeName(std::ostream& out) const {
    out << "-" << west_east_ << "x" << south_north_;
}


}  // namespace mir::util
