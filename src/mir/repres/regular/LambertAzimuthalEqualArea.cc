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


#include "mir/repres/regular/LambertAzimuthalEqualArea.h"

#include <cmath>

#include "eckit/geo/Projection.h"
#include "eckit/geo/figure/Earth.h"
#include "eckit/geo/spec/Custom.h"

#include "mir/param/MIRParametrisation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Grib.h"


namespace mir::repres::regular {


static const RepresentationBuilder<LambertAzimuthalEqualArea> __builder("lambert_azimuthal_equal_area");


RegularGrid::Projection* make_projection_laea(const param::MIRParametrisation& param) {
    if (const auto* proj = RegularGrid::make_projection(param); proj != nullptr) {
        return proj;
    }

    double standardParallel = 0.;
    double centralLongitude = 0.;
    double radius           = 0.;
    ASSERT(param.get("standardParallelInDegrees", standardParallel));
    ASSERT(param.get("centralLongitudeInDegrees", centralLongitude));
    param.get("radius", radius = ::eckit::geo::figure::EARTH.R());

    using spec_type = ::eckit::geo::spec::Custom;
    return ::eckit::geo::ProjectionFactory::build(
        *std::unique_ptr<spec_type>(new spec_type{{"type", "lambert_azimuthal_equal_area"},
                                                  {"standard_parallel", standardParallel},
                                                  {"central_longitude", centralLongitude},
                                                  {"radius", radius}}));
}


LambertAzimuthalEqualArea::LambertAzimuthalEqualArea(const param::MIRParametrisation& param) :
    RegularGrid(param, make_projection_laea(param)) {}


void LambertAzimuthalEqualArea::fillGrib(grib_info& info) const {
    info.grid.grid_type        = CODES_UTIL_GRID_SPEC_LAMBERT_AZIMUTHAL_EQUAL_AREA;
    info.packing.editionNumber = 2;

    auto firstLL   = firstPointLonLat();
    auto reference = referencePointLonLat();

    info.grid.Ni = static_cast<long>(x().size());
    info.grid.Nj = static_cast<long>(y().size());

    info.grid.latitudeOfFirstGridPointInDegrees  = firstLL.lat;
    info.grid.longitudeOfFirstGridPointInDegrees = firstLL.lon;

    info.extra_set("DxInMetres", std::abs(static_cast<double>(grid().x().increment())));
    info.extra_set("DyInMetres", std::abs(static_cast<double>(grid().y().increment())));
    info.extra_set("standardParallelInDegrees", reference.lat);
    info.extra_set("centralLongitudeInDegrees", reference.lon);

    // some extra keys are edition-specific, so parent call is here
    RegularGrid::fillGrib(info);
}


}  // namespace mir::repres::regular
