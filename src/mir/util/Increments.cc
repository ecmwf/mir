// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/util/Increments.h"

#include <ostream>

#include "eckit/log/JSON.h"
#include "eckit/spec/Custom.h"
#include "eckit/types/FloatCompare.h"

#include "mir/api/MIRJob.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/util/BoundingBox.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Grib.h"


namespace mir::util {


Increments::Increments() : inc_{0., 0.} {}


Increments::Increments(const param::MIRParametrisation& parametrisation) : inc_{0., 0.} {
    double south_north = 0.;
    ASSERT(parametrisation.get("south_north_increment", south_north));

    double west_east = 0.;
    ASSERT(parametrisation.get("west_east_increment", west_east));

    inc_ = {west_east, south_north};
}


Increments::Increments(const Increments&) = default;


Increments::Increments(double westEastIncrement, double southNorthIncrement) :
    inc_{westEastIncrement, southNorthIncrement} {}


bool Increments::operator==(const Increments& other) const {
    return eckit::types::is_approximately_equal(west_east(), other.west_east(), PointLonLat::EPS) &&
           eckit::types::is_approximately_equal(south_north(), other.south_north(), PointLonLat::EPS);
}


bool Increments::operator!=(const Increments& other) const {
    return !operator==(other);
}


Increments& Increments::operator=(const Increments&) = default;


bool Increments::isPeriodic() const {
    return inc_.periodic();
}


bool Increments::isShifted(const BoundingBox& bbox) const {
    return isShifted(PointLonLat{bbox.west(), bbox.south()});
}


bool Increments::isLatitudeShifted(const BoundingBox& bbox) const {
    return inc_.lat_shifted(bbox.south());
}


bool Increments::isLongitudeShifted(const BoundingBox& bbox) const {
    return inc_.lon_shifted(bbox.west());
}


bool Increments::isShifted(const PointLonLat& p) const {
    return inc_.shifted(p);
}


bool Increments::isLatitudeShifted(const PointLonLat& p) const {
    return inc_.lat_shifted(p.lat());
}


bool Increments::isLongitudeShifted(const PointLonLat& p) const {
    return inc_.lon_shifted(p.lon());
}


void Increments::print(std::ostream& out) const {
    out << "Increments["
        << "west_east=" << west_east() << ",south_north=" << south_north() << "]";
}

void Increments::json(eckit::JSON& json) const {
    json.startObject();
    json << "west_east" << west_east();
    json << "south_north" << south_north();
    json.endObject();
}


void Increments::fillGrib(grib_info& info) const {
    // Warning: scanning mode not considered
    info.grid.iDirectionIncrementInDegrees = west_east();
    info.grid.jDirectionIncrementInDegrees = south_north();
}


void Increments::fillJob(api::MIRJob& job) const {
    job.set("grid", west_east(), south_north());
}


void Increments::fillSpec(eckit::spec::Custom& spec) const {
    spec.set("grid", std::vector<double>{west_east(), south_north()});
}


void Increments::makeName(std::ostream& out) const {
    out << "-" << west_east() << "x" << south_north();
}


}  // namespace mir::util
