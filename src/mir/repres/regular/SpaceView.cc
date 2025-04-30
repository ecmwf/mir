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


#include "mir/repres/regular/SpaceView.h"

#include <cmath>
#include <string>
#include <vector>

#include "eckit/geo/Projection.h"
#include "eckit/geo/spec/Custom.h"
#include "eckit/types/FloatCompare.h"

#include "mir/iterator/UnstructuredIterator.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Iterator.h"
#include "mir/util/Angles.h"
#include "mir/util/Exceptions.h"


namespace mir::repres::regular {


static const RepresentationBuilder<SpaceView> __builder("space_view");



template <typename EXTERNAL_T, typename INTERNAL_T = EXTERNAL_T>
EXTERNAL_T get(const param::MIRParametrisation& param, const std::string& key) {
    INTERNAL_T value;
    ASSERT(param.get(key, value));
    return static_cast<EXTERNAL_T>(value);
}


RegularGrid::Projection* make_projection(const param::MIRParametrisation& param) {
    auto earthIsOblate = get<bool>(param, "earthIsOblate");
    auto a             = get<double>(param, earthIsOblate ? "earthMajorAxis" : "radius");
    auto b             = earthIsOblate ? get<double>(param, "earthMinorAxis") : a;

    auto Nr = get<double>(param, "NrInRadiusOfEarthScaled");
    ASSERT(Nr > 1.);

    auto h = (Nr - 1.) * a;

    auto Lop = get<double>(param, "longitudeOfSubSatellitePointInDegrees");
    auto Lap = get<double>(param, "latitudeOfSubSatellitePointInDegrees");
    ASSERT(eckit::types::is_approximately_equal(Lap, 0.));
    ASSERT(get<size_t>(param, "orientationOfTheGridInDegrees") == 180);

    std::string proj = "+proj=geos +type=crs +sweep=y";
    proj += " +h=" + std::to_string(h);
    if (!eckit::types::is_approximately_equal(Lop, 0.)) {
        proj += " +lon_0=" + std::to_string(Lop);
    }
    if (eckit::types::is_approximately_equal(a, b)) {
        proj += " +R=" + std::to_string(a);
    }
    else {
        proj += " +a=" + std::to_string(a) + " +b=" + std::to_string(b);
    }

    using spec_type = ::eckit::geo::spec::Custom;
    return ::eckit::geo::ProjectionFactory::build(
        *std::unique_ptr<spec_type>(new spec_type{{"type", "proj"}, {"proj", proj}}));
}


SpaceView::SpaceView(const param::MIRParametrisation& param) : RegularGrid(param, make_projection(param)) {
    auto earthIsOblate = get<bool>(param, "earthIsOblate");
    auto a             = get<double>(param, earthIsOblate ? "earthMajorAxis" : "radius");

    auto Nr = get<double>(param, "NrInRadiusOfEarthScaled");
    ASSERT(Nr > 1.);

    auto h = (Nr - 1.) * a;

    // --

    // x space
    auto Nx_ = get<long>(param, "Nx");
    ASSERT(1 < Nx_);

    auto ip = get<bool>(param, "iScansPositively");
    auto xp = get<double, long>(param, "XpInGridLengths");
    auto dx = get<double, long>(param, "dx");
    ASSERT(dx > 0);

    auto rx = 2. * std::asin(1. / Nr) / dx * h;  // (height factor is PROJ-specific)

    double xa_       = 0;
    double xb_       = 0;
    (ip ? xa_ : xb_) = rx * (-xp);
    (ip ? xb_ : xa_) = rx * (-xp + static_cast<double>(Nx_ - 1));

    // RegularGrid::LinearSpacing x() const { return {xa_, xb_, Nx_, true}; }

    // y space
    auto Ny_ = get<long>(param, "Ny");
    ASSERT(1 < Ny_);

    auto jp = get<bool>(param, "jScansPositively");
    auto yp = get<double, long>(param, "YpInGridLengths");
    auto dy = get<double, long>(param, "dy");
    ASSERT(dy > 0);

    auto ry = 2. * std::asin(1. / Nr) / dy * h;  // (height factor is PROJ-specific)

    double ya_       = 0;
    double yb_       = 0;
    (jp ? ya_ : yb_) = ry * (-yp);
    (jp ? yb_ : ya_) = ry * (-yp + static_cast<double>(Ny_ - 1));

    // RegularGrid::LinearSpacing y() const { return {ya_, yb_, Ny_, true}; }

    // bounding box
    // [1] page 25, solution of s_d^2=0, restrained at x=0 (lon) and y=0 (lat). Note: uses a, b, height defined there
    auto eps_ll = 1e-6;

    auto n = 90. - util::radian_to_degree(0.151347) + eps_ll;
    auto s = -n;

    auto e = 90. - util::radian_to_degree(0.151853) + eps_ll + Lop_;
    auto w = 2. * Lop_ - e;

    bbox_ = {n, w, s, e};
}


void SpaceView::fillGrib(grib_info& /*info*/) const {
    NOTIMP;
}


Iterator* SpaceView::iterator() const {
    ASSERT(!longitudes_.empty() && !latitudes_.empty() && longitudes_.size() == latitudes_.size());
    return new iterator::UnstructuredIterator(latitudes_, longitudes_);
}


}  // namespace mir::repres::regular
