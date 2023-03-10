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


#include "mir/repres/Gridded.h"

#include "mir/api/MIREstimation.h"
#include "mir/util/Domain.h"
#include "mir/util/Grib.h"


namespace mir::repres {


Gridded::Gridded(const param::MIRParametrisation& parametrisation) : bbox_(parametrisation) {}


Gridded::Gridded(const util::BoundingBox& bbox) : bbox_(bbox) {}


Gridded::~Gridded() = default;


Gridded::Gridded() = default;


util::Domain Gridded::domain() const {
    const Latitude& n  = includesNorthPole() ? Latitude::NORTH_POLE : bbox_.north();
    const Latitude& s  = includesSouthPole() ? Latitude::SOUTH_POLE : bbox_.south();
    const Longitude& w = bbox_.west();
    const Longitude& e = isPeriodicWestEast() ? bbox_.west() + Longitude::GLOBE : bbox_.east();

    return util::Domain(n, w, s, e);
}


const util::BoundingBox& Gridded::boundingBox() const {
    return bbox_;
}


bool Gridded::getLongestElementDiagonal(double& /*unused*/) const {
    return false;
}


void Gridded::estimate(api::MIREstimation& estimation) const {
    estimation.packing("grid_simple");  // Will be overriden
    estimation.representation(factory());
}


bool Gridded::crop(util::BoundingBox& /*unused*/, util::IndexMapping& /*unused*/) const {
    return false;
}


size_t Gridded::numberOfValues() const {
    return numberOfPoints();
}


}  // namespace mir::repres
