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

#include "mir/param/MIRParametrisation.h"
#include "mir/util/Exceptions.h"


namespace mir::repres::regular {


static const RepresentationBuilder<PolarStereographic> __builder("polar_stereographic");


PolarStereographic::PolarStereographic(const param::MIRParametrisation& param) :
    RegularGrid(param, make_proj_spec(param)) {
    // ensure bounding box covers either pole
    long sp = 0;
    ASSERT(param.get("southPoleOnProjectionPlane", sp));

    bbox_ = {sp == 0 ? Latitude::NORTH_POLE : bbox_.north(), bbox_.west(),
             sp == 1 ? Latitude::SOUTH_POLE : bbox_.south(), bbox_.east()};
}


bool PolarStereographic::isPeriodicWestEast() const {
    // periodicity implied by pole
    return true;
}


void PolarStereographic::fillGrib(grib_info& /*info*/) const {
    NOTIMP;
}


}  // namespace mir::repres::regular
