/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Pedro Maciel
/// @date Apr 2015


#include "mir/action/filter/AdjustWindsDirections.h"

#include <cmath>
#include <iostream>
#include "eckit/exception/Exceptions.h"
#include "eckit/memory/ScopedPtr.h"
#include "mir/action/context/Context.h"
#include "mir/data/MIRField.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Iterator.h"
#include "mir/repres/Representation.h"
#include "mir/util/Angles.h"
#include "mir/util/Compare.h"


namespace mir {
namespace action {


AdjustWindsDirections::AdjustWindsDirections(const param::MIRParametrisation &parametrisation):
    Action(parametrisation) {

    std::vector<double> value;
    ASSERT(parametrisation_.get("user.rotation", value));
    ASSERT(value.size() == 2);

    rotation_ = util::Rotation(value[0], value[1]);
}


AdjustWindsDirections::~AdjustWindsDirections() {
}


bool AdjustWindsDirections::sameAs(const Action& other) const {
    const AdjustWindsDirections* o = dynamic_cast<const AdjustWindsDirections*>(&other);
    return o && (rotation_ == o->rotation_);
}


void AdjustWindsDirections::print(std::ostream &out) const {
    out << "AdjustWindsDirections[rotation=" << rotation_ << "]";
}


inline double normalize(double x) {
    return std::max(std::min(x, 1.0), -1.0);
}


inline double sign(double a, double b) {
    if (b >= 0.0 ) {
        return fabs(a);
    }
    return -fabs(a);
}


void AdjustWindsDirections::windDirections(const repres::Representation* representation, std::vector<double> &result) const {

    ASSERT(representation);

    result.clear();

    // eckit::Log::info() << "AdjustWindsDirections::windDirections " << *representation << std::endl;

    eckit::ScopedPtr<repres::Iterator> iter(representation->rotatedIterator());

    // eckit::Log::info() << "AdjustWindsDirections::windDirections " << *iter << std::endl;


    Latitude lat = 0;
    Longitude lon = 0;

    // Inspired from HPSHGPW

    double pole_longitude = -rotation_.south_pole_longitude();
    double theta = util::angles::degree_to_radian(rotation_.south_pole_latitude());
    double sin_theta = -sin(theta);
    double cos_theta = -cos(theta);

    while (iter->next(lat, lon)) {

        double radian_lat = util::angles::degree_to_radian(lat);
        double sin_lat = sin(radian_lat);
        double cos_lat = cos(radian_lat);

        lon += pole_longitude;

        // For some reason, the algorithms only work between in ]-180,180]
        lon = util::angles::between_m180_and_p180(lon);
        if (lon == Longitude::MINUS_DATE_LINE) {
            lon = Longitude::DATE_LINE;
        }

        double radian_lon = util::angles::degree_to_radian(lon);
        double sin_lon = sin(radian_lon);
        double cos_lon = cos(radian_lon);
        double z = normalize(sin_theta * sin_lat + cos_theta * cos_lat * cos_lon);

        double ncos_lat = 0;

        if ( !(eckit::types::is_approximately_equal<double>(z, 1.0) ||
                eckit::types::is_approximately_equal<double>(z, -1.0))) {
            ncos_lat = cos(asin(z));
        }

        if (eckit::types::is_approximately_equal<double>(ncos_lat, 0.0)) {
            ncos_lat = 1.0;
        }

        double cos_new = normalize(( (sin_theta * cos_lat * cos_lon - cos_theta * sin_lat) ) / ncos_lat);
        double lon_new = sign(acos(cos_new), radian_lon);

        double cos_delta = normalize(sin_theta * sin_lon * sin(lon_new) + cos_lon * cos_new);
        double delta = sign(acos(cos_delta), -cos_theta * radian_lon);

        result.push_back(delta);
    }

}


void AdjustWindsDirections::execute(context::Context & ctx) const {
    data::MIRField& field = ctx.field();

    ASSERT((field.dimensions() % 2) == 0);

    std::vector<double> directions;

    size_t size = field.values(0).size();
    directions.reserve(field.values(0).size());

    ASSERT(!field.hasMissing()); // For now

    windDirections(field.representation(), directions);

    if (directions.size() != size) {
        eckit::Log::info() << "AdjustWindsDirections::windDirections after=" << directions.size()
                  << " before=" << size << std::endl;

    }
    ASSERT(directions.size() == size);


    std::vector<double> c(size);
    std::vector<double> s(size);

    for (size_t i = 0; i < size; i++) {
        double d =  directions[i];
        c[i] = cos(d);
        s[i] = sin(d);
    }

    for (size_t i = 0; i < field.dimensions(); i += 2 ) {

        // TODO: use matrix multiplication

        const std::vector<double> &u_values = field.values(i);
        const std::vector<double> &v_values = field.values(i + 1);

        ASSERT(u_values.size() == size);
        ASSERT(v_values.size() == size);

        std::vector<double> new_u_values(size);
        std::vector<double> new_v_values(size);

        for (size_t j = 0; j < size; j++) {
            new_u_values[j] = u_values[j] * c[j] - v_values[j] * s[j];
            new_v_values[j] = u_values[j] * s[j] + v_values[j] * c[j];
        }

        field.update(new_u_values, i);
        field.update(new_v_values, i + 1);
    }
}


namespace {
static ActionBuilder< AdjustWindsDirections > filter("filter.adjust-winds-directions");
}


}  // namespace action
}  // namespace mir

