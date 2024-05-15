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


#include "mir/repres/regular/PolarStereographic.h"

#include <cmath>
#include <sstream>

#include "eckit/types/FloatCompare.h"

#include "mir/api/MIRJob.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/util/Angles.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Grib.h"


namespace mir::repres::regular {


static const RepresentationBuilder<PolarStereographic> __builder("polar_stereographic");


PolarStereographic::PolarStereographic(const param::MIRParametrisation& param) :
    RegularGrid(param, make_proj_spec(param)),
    LaDInDegrees_(0),
    orientationOfTheGridInDegrees_(0),
    southPoleOnProjectionPlane_(false) {
    ASSERT(param.get("proj", proj_) && !proj_.empty());

    param.get("LaDInDegrees", LaDInDegrees_);
    param.get("orientationOfTheGridInDegrees", orientationOfTheGridInDegrees_);

    long south = 0;
    param.get("southPoleOnProjectionPlane", south);
    southPoleOnProjectionPlane_ = south != 0;
}


bool PolarStereographic::isPeriodicWestEast() const {
    return eckit::types::is_approximately_equal(bbox_.north().value(), Latitude::NORTH_POLE.value()) ||
           eckit::types::is_approximately_equal(bbox_.south().value(), Latitude::SOUTH_POLE.value()) ||
           eckit::types::is_approximately_equal(bbox_.east().value() - bbox_.west().value(), Longitude::GLOBE.value());
}


void PolarStereographic::fillGrib(grib_info& info) const {
    info.grid.grid_type = CODES_UTIL_GRID_SPEC_POLAR_STEREOGRAPHIC;
    info.extra_set("edition", 2L);  // write LaDInDegrees

    Point2 first   = {x_.front(), y_.front()};
    Point2 firstLL = grid_.projection().lonlat(first);

    info.grid.latitudeOfFirstGridPointInDegrees  = firstLL[LLCOORDS::LAT];
    info.grid.longitudeOfFirstGridPointInDegrees = util::normalise_longitude(firstLL[LLCOORDS::LON], 0);

    info.grid.Ni = static_cast<long>(x_.size());
    info.grid.Nj = static_cast<long>(y_.size());

    info.grid.iScansNegatively = x_.back() < x_.front() ? 1L : 0L;
    info.grid.jScansPositively = y_.front() < y_.back() ? 1L : 0L;

    info.extra_set("DxInMetres", std::abs(x_.step()));
    info.extra_set("DyInMetres", std::abs(y_.step()));
    info.extra_set("LaDInDegrees", LaDInDegrees_);
    info.extra_set("orientationOfTheGridInDegrees", util::normalise_longitude(orientationOfTheGridInDegrees_, 0));

    // some extra keys are edition-specific, so parent call is here
    RegularGrid::fillGrib(info);
}


void PolarStereographic::fillJob(api::MIRJob& job) const {
    Point2 first   = {x_.front(), y_.front()};
    Point2 firstLL = grid_.projection().lonlat(first);

    std::ostringstream grid;
    grid << "gridType=polar_stereographic;"
         << "proj=" << proj_ << ";"
         << "LaDInDegrees=" << LaDInDegrees_ << ";"
         << "orientationOfTheGridInDegrees=" << orientationOfTheGridInDegrees_ << ";"
         << "southPoleOnProjectionPlane=" << (southPoleOnProjectionPlane_ ? "1" : "0") << ";"
         << "Ni=" << x_->size() << ";"
         << "Nj=" << y_->size() << ";"
         << "grid=" << std::abs(x_.step()) << "/" << std::abs(y_.step()) << ";"
         << "latitudeOfFirstGridPointInDegrees=" << firstLL[LLCOORDS::LAT] << ";"
         << "longitudeOfFirstGridPointInDegrees=" << firstLL[LLCOORDS::LON] << ";"
         << "iScansNegatively=" << (x_.front() < x_.back() ? 0 : 1) << ";"
         << "jScansPositively=" << (y_.front() < y_.back() ? 1 : 0);

    if (shape().provided) {
        grid << ";shapeOfTheEarth=" << shape().code << ";earthMajorAxis=" << shape().a
             << ";earthMinorAxis=" << shape().b;
    }

    job.set("grid", grid.str());
}


}  // namespace mir::repres::regular
