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
#include <memory>

#include "eckit/geo/spec/Custom.h"

#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Iterator.h"
#include "mir/util/Earth.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Grib.h"


namespace mir::repres::regular {


static const RepresentationBuilder<LambertAzimuthalEqualArea> __builder("lambert_azimuthal_equal_area");


LambertAzimuthalEqualArea::LambertAzimuthalEqualArea(const param::MIRParametrisation& param) :
    RegularGrid(param, make_projection(param)) {}


eckit::geo::spec::Custom* LambertAzimuthalEqualArea::make_projection(const param::MIRParametrisation& param) {
    if (auto* spec = make_proj_spec(param); spec != nullptr) {
        return spec;
    }

    double standardParallel = 0.;
    double centralLongitude = 0.;
    double radius           = 0.;
    ASSERT(param.get("standardParallelInDegrees", standardParallel));
    ASSERT(param.get("centralLongitudeInDegrees", centralLongitude));
    param.get("radius", radius = util::Earth::radius());

    return new eckit::geo::spec::Custom{{"type", "lambert_azimuthal_equal_area"},
                                        {"standard_parallel", standardParallel},
                                        {"central_longitude", centralLongitude},
                                        {"radius", radius}};
}


void LambertAzimuthalEqualArea::fillGrib(grib_info& info) const {
    info.grid.grid_type        = CODES_UTIL_GRID_SPEC_LAMBERT_AZIMUTHAL_EQUAL_AREA;
    info.packing.editionNumber = 2;

    auto reference = std::get<PointLonLat>(projection().inv(Point2{0., 0.}));
    auto firstLL   = std::get<PointLonLat>(projection().inv(Point2{x().front(), y().front()}));

    info.grid.Ni = static_cast<long>(x().size());
    info.grid.Nj = static_cast<long>(y().size());

    info.grid.latitudeOfFirstGridPointInDegrees  = firstLL.lat;
    info.grid.longitudeOfFirstGridPointInDegrees = firstLL.lon;

    info.extra_set("DxInMetres", std::abs(x().step()));
    info.extra_set("DyInMetres", std::abs(y().step()));
    info.extra_set("standardParallelInDegrees", reference.lat);
    info.extra_set("centralLongitudeInDegrees", reference.lon);

    // some extra keys are edition-specific, so parent call is here
    RegularGrid::fillGrib(info);
}


}  // namespace mir::repres::regular
