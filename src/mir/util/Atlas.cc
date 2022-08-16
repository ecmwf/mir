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


#include "mir/util/Atlas.h"
#if mir_HAVE_ATLAS
#error "mir/util/Atlas.cc cannot be included with Atlas present"
#endif

#include <algorithm>

#include "eckit/types/FloatCompare.h"
#include "eckit/types/Fraction.h"
#include "eckit/utils/Translator.h"

#include "mir/util/Exceptions.h"
#include "mir/util/Grib.h"


namespace atlas {


Domain::Range::Range(double min, double max) : min_(min), max_(max) {
    ASSERT(min <= max);
}


Domain::Domain(Domain::Range&& lon, Domain::Range&& lat, const std::string&) : lon_(lon), lat_(lat) {
    ASSERT(lon_.max_ - lon_.min_ <= mir::Longitude::GLOBE.value());
    ASSERT(mir::Latitude::SOUTH_POLE.value() <= lat_.min_ && lat_.max_ <= mir::Latitude::NORTH_POLE.value());
}


bool Domain::zonal_band() const {
    return eckit::types::is_approximately_equal(east() - west(), mir::Longitude::GLOBE.value());
}


bool Domain::global() const {
    return eckit::types::is_approximately_equal(south(), mir::Latitude::SOUTH_POLE.value()) &&
           eckit::types::is_approximately_equal(north(), mir::Latitude::NORTH_POLE.value()) && zonal_band();
}


bool Domain::operator==(const Domain& other) const {
    return mir::Latitude(north() - other.north()) == 0 && mir::Latitude(south() - other.south()) == 0 &&
           mir::Longitude(west() - other.west()) == 0 && mir::Longitude(east() - other.east()) == 0;
}


bool projection::ProjectionFactory::has(const std::string&) {
    return false;
}


void util::gaussian_latitudes_npole_equator(size_t N, double* latitudes) {
    std::vector<double> tmp(2 * N);
    codes_get_gaussian_latitudes(static_cast<long>(N), tmp.data());
    std::copy_n(tmp.begin(), N, latitudes);
}


void util::gaussian_latitudes_npole_spole(size_t N, double* latitudes) {
    codes_get_gaussian_latitudes(static_cast<long>(N), latitudes);
}


void util::gaussian_quadrature_npole_spole(size_t /*N*/, double* /*latitudes*/, double* /*weights*/) {
    // used in interpolation=grid-box-average/maximum
    NOTIMP;
}


util::Rotation::Rotation(const PointLonLat& southPole) :
    PointLonLat(mir::Longitude::GREENWICH.value(), mir::Latitude::SOUTH_POLE.value()) {
    ASSERT(southPole == *this);
}


namespace grid {
LinearSpacing::LinearSpacing(value_type a, value_type b, long n, bool endpoint) : Spacing(static_cast<size_t>(n)) {
    ASSERT(n > 1);
    eckit::Fraction dx((b - a) / static_cast<double>(n - (endpoint ? 1 : 0)));
    eckit::Fraction x(a);

    resize(static_cast<size_t>(n));
    for (auto& v : *this) {
        v = x;
        x += dx;
    }
}
}  // namespace grid


Grid::Projection Grid::projection() const {
    NOTIMP;
}


StructuredGrid::StructuredGrid(const Grid&) {
    NOTIMP;
}


idx_t StructuredGrid::nx() const {
    ASSERT(!pl_.empty());
    auto mm = std::minmax_element(pl_.begin(), pl_.end());
    ASSERT(*mm.first == *mm.second);
    return *mm.first;
}


GaussianGrid::GaussianGrid(const std::string& name, const Domain& domain) {
    ASSERT(domain == Domain());
    ASSERT(name.size() > 1);
    spec_.set("name", name);

    auto c = name.front();
    auto n = eckit::Translator<std::string, idx_t>()(name.substr(1));
    ASSERT(n > 0);

    if (c == 'F' || c == 'f') {
        pl_.assign(static_cast<size_t>(2 * n), 4 * n);
        return;
    }

    if (c == 'O' || c == 'o') {
        pl_.resize(static_cast<size_t>(2 * n));

        auto a = pl_.begin();
        auto b = pl_.rbegin();
        for (idx_t j = 0, ni = 20; j < n; j++, ni += 4) {
            *(a++) = *(b++) = ni;
        }
        return;
    }

    NOTIMP;
}


GaussianGrid::GaussianGrid(const std::vector<long>& pl, const Domain& domain) {
    ASSERT(domain == Domain());
    pl_ = pl;
}


UnstructuredGrid::UnstructuredGrid(std::vector<PointXY>&& points) : points_(points) {}


trans::LegendreCache::LegendreCache(const void*, size_t) {
    NOTIMP;
}


}  // namespace atlas
