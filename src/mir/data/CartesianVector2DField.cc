/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/data/CartesianVector2DField.h"

//#include <complex>
#include <cmath>
#include <iostream>
#include "eckit/exception/Exceptions.h"
#include "eckit/types/FloatCompare.h"
#include "mir/api/Atlas.h"
#include "mir/api/MIRJob.h"
#include "mir/api/mir_config.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Iterator.h"
#include "mir/repres/Representation.h"
#include "mir/util/Angles.h"
#include "mir/util/Rotation.h"


namespace mir {
namespace data {


namespace {

inline double normalize(double x) {
    return std::max(std::min(x, 1.0), -1.0);
}

inline double sign(double a, double b) {
    if (b >= 0.0 ) {
        return fabs(a);
    }
    return -fabs(a);
}

}  // (anonymous namespace)


CartesianVector2DField::CartesianVector2DField(
        const repres::Representation* representation,
        bool hasMissing,
        double missingValue) :
    valuesX_(),
    valuesY_(),
    hasMissing_(hasMissing),
    missingValue_(missingValue),
    representation_(representation) {

    ASSERT(representation_);
    representation_->attach();
}


CartesianVector2DField::~CartesianVector2DField() {
}


void CartesianVector2DField::rotate(const util::Rotation& r) {
    std::vector<double> dummyX, dummyY;
    rotate(r, dummyX, dummyY);
}


void CartesianVector2DField::rotate(const util::Rotation& r, std::vector<double>& valuesX, std::vector<double>& valuesY) const {

//    // setup Atlas rotation
//    const std::vector<double> southPole = {
//        r.south_pole_longitude().value(),
//        r.south_pole_latitude().value()
//    };

//    atlas::util::Config config;
//    config.set("type", "rotated_lonlat");
//    config.set("south_pole", southPole);
//    config.set("rotation_angle", r.south_pole_rotation_angle());
//    atlas::Projection projection = atlas::Projection(config);


    // setup results vectors
    ASSERT(valuesX.size() == valuesY.size());
    const size_t N(valuesX.size());

    ASSERT(!hasMissing_); // For now


    // Inspired from HPSHGPW

    std::vector<double> directions(N);

    double pole_longitude = -r.south_pole_longitude().value();
    double theta = util::angles::degree_to_radian(r.south_pole_latitude().value());
    double sin_theta = -std::sin(theta);
    double cos_theta = -std::cos(theta);

    eckit::ScopedPtr<repres::Iterator> it(representation_->iterator());
    size_t ip = 0;

    while (it->next()) {
        ASSERT(ip < N);

        const repres::Iterator::point_ll_t& p = it->pointUnrotated();

        double radian_lat = util::angles::degree_to_radian(p.lat.value());
        double sin_lat = std::sin(radian_lat);
        double cos_lat = std::cos(radian_lat);

        // For some reason, the algorithms only work between in ]-180,180] or [-180,180[
        Longitude lon = p.lon + pole_longitude;
        lon = lon.normalise(Longitude::MINUS_DATE_LINE);

        double radian_lon = util::angles::degree_to_radian(lon.value());
        double sin_lon = std::sin(radian_lon);
        double cos_lon = std::cos(radian_lon);
        double z = normalize(sin_theta * sin_lat + cos_theta * cos_lat * cos_lon);

        double ncos_lat = 0;

        if (!(eckit::types::is_approximately_equal(z,  1.0) ||
              eckit::types::is_approximately_equal(z, -1.0))) {
            ncos_lat = std::cos(std::asin(z));
        }

        if (eckit::types::is_approximately_equal(ncos_lat, 0.0)) {
            ncos_lat = 1.0;
        }

        double cos_new = normalize(( (sin_theta * cos_lat * cos_lon - cos_theta * sin_lat) ) / ncos_lat);
        double lon_new = sign(std::acos(cos_new), radian_lon);

        double cos_delta = normalize(sin_theta * sin_lon * std::sin(lon_new) + cos_lon * cos_new);
        double delta = sign(std::acos(cos_delta), -cos_theta * radian_lon);

        directions[ip] = delta;
        ++ip;
    }


    std::vector<double> c(N);
    std::vector<double> s(N);

    for (size_t i = 0; i < N; i++) {
        double d =  directions[i];
        c[i] = std::cos(d);
        s[i] = std::sin(d);
    }



    // TODO: use matrix multiplication

    std::vector<double> resultX(N);
    std::vector<double> resultY(N);

    for (size_t j = 0; j < N; j++) {
        resultX[j] = valuesX[j] * c[j] - valuesY[j] * s[j];
        resultY[j] = valuesX[j] * s[j] + valuesY[j] * c[j];
    }

    valuesX.swap(resultX);
    valuesY.swap(resultY);
}


void CartesianVector2DField::print(std::ostream& out) const {
    out << "CartesianVector2DField["
        << "]";
}


}  // namespace data
}  // namespace mir

