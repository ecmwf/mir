/*
 * (C) Copyright 1996-2016 ECMWF.
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

#include "mir/util/Domain.h"
#include "mir/util/Grib.h"


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


void Gridded::setComplexPacking(grib_info &info) const {
    info.packing.packing_type = GRIB_UTIL_PACKING_TYPE_GRID_COMPLEX;
}


void Gridded::setSimplePacking(grib_info &info) const {
    info.packing.packing_type = GRIB_UTIL_PACKING_TYPE_GRID_SIMPLE;
}


void Gridded::setGivenPacking(grib_info &info) const {
    // The packing_type is set by the caller
}


void Gridded::cropToDomain(const param::MIRParametrisation &parametrisation, context::Context & ctx) const {
    if (!domain().isGlobal()) {
        Representation::cropToDomain(parametrisation, ctx); // This will throw an exception
    }
}


util::Domain Gridded::domain() const {
    using value_t = util::BoundingBox::value_type;

    const value_t& n = includesNorthPole()? util::BoundingBox::NORTH_POLE : bbox_.north();
    const value_t& s = includesSouthPole()? util::BoundingBox::SOUTH_POLE : bbox_.south();
    const value_t& w = bbox_.west();
    const value_t& e = isPeriodicWestEast()? bbox_.west() + util::BoundingBox::THREE_SIXTY : bbox_.east();

    return util::Domain(n, w, s, e);
}


}  // namespace repres
}  // namespace mir

