/*
 * (C) Copyright 1996- ECMWF.
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


#include "mir/repres/Gridded.h"

#include "eckit/log/Log.h"
#include "mir/action/misc/AreaCropper.h"
#include "mir/util/Domain.h"
#include "mir/util/Grib.h"
#include "mir/config/LibMir.h"


namespace mir {
namespace repres {


Gridded::Gridded() {}


Gridded::Gridded(const param::MIRParametrisation& parametrisation) :
    bbox_(parametrisation) {
}


Gridded::Gridded(const util::BoundingBox& bbox) :
    bbox_(bbox) {
}


Gridded::~Gridded() {}


void Gridded::setComplexPacking(grib_info& info) const {
    info.packing.packing_type = GRIB_UTIL_PACKING_TYPE_GRID_COMPLEX;
}


void Gridded::setSimplePacking(grib_info& info) const {
    info.packing.packing_type = GRIB_UTIL_PACKING_TYPE_GRID_SIMPLE;
}


void Gridded::setGivenPacking(grib_info&) const {
    // The packing_type is set by the caller
}


void Gridded::crop(const param::MIRParametrisation& parametrisation, context::Context& ctx) const {
    // only crop if not global
    if (!isGlobal()) {
        std::cout << "+++++++++++ " << *this << " is not global" << std::endl;
        action::AreaCropper cropper(parametrisation, bbox_);
        cropper.execute(ctx);
    }
}


util::Domain Gridded::domain() const {

    const Latitude& n = includesNorthPole()? Latitude::NORTH_POLE : bbox_.north();
    const Latitude& s = includesSouthPole()? Latitude::SOUTH_POLE : bbox_.south();
    const Longitude& w = bbox_.west();
    const Longitude& e = isPeriodicWestEast()? bbox_.west() + Longitude::GLOBE : bbox_.east();

    return util::Domain(n, w, s, e);
}


void Gridded::adjustBoundingBox(util::BoundingBox&) const{
    // normally, no adjustments are necessary
}


bool Gridded::getLongestElementDiagonal(double&) const {
    return false;
}


}  // namespace repres
}  // namespace mir

