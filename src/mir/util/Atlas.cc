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


#include "mir/util/Types.h"

#include <algorithm>

#include "eckit/exception/Exceptions.h"
#include "eckit/utils/Translator.h"

#include "mir/util/Angles.h"
#include "mir/util/Grib.h"


namespace atlas {


Domain::Range::Range(double min, double max) : min_(min), max_(max) {
    ASSERT(min <= max);
}


Domain::Domain(Domain::Range&& lon, Domain::Range&& lat, std::string) : lon_(lon), lat_(lat) {
    ASSERT(lon_.max_ - lon_.min_ <= mir::Longitude::GLOBE.value());
    ASSERT(mir::Latitude::SOUTH_POLE.value() <= lat_.min_ && lat_.max_ <= mir::Latitude::NORTH_POLE.value());
}


bool projection::ProjectionFactory::has(const std::string&) {
    return false;
}


void util::gaussian_latitudes_npole_spole(size_t N, double* latitudes) {
    codes_get_gaussian_latitudes(long(N), latitudes);
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
LinearSpacing::LinearSpacing(value_type /*a*/, value_type /*b*/, long n, bool /*endpoint*/) : Spacing(n) {
    NOTIMP;
}
}  // namespace grid


Projection::Projection(const Projection::Spec& spec) : spec_(spec) {
    ASSERT(spec.empty());
}


StructuredGrid::StructuredGrid(const std::string& name, const Domain& domain) {
    ASSERT(domain.north() == globalDomain.north());
    ASSERT(domain.south() == globalDomain.south());
    ASSERT(domain.west() == globalDomain.west());
    ASSERT(domain.east() == globalDomain.east());

    ASSERT(name.size() > 1);
    auto c = name.front();
    auto n = eckit::Translator<std::string, idx_t>()(name.substr(1));
    ASSERT(n > 0);

    if (c == 'F' || c == 'f') {
        pl_.assign(size_t(2 * n), 4 * n);
        return;
    }

    if (c == 'O' || c == 'o') {
        pl_.resize(size_t(2 * n));

        auto a = pl_.begin();
        auto b = pl_.rbegin();
        for (idx_t j = 0, ni = 20; j < n; j++, ni += 4) {
            *(a++) = *(b++) = ni;
        }
        return;
    }

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


Grid::Grid(const Grid::Spec& spec) : spec_(spec) {
    NOTIMP;
}


Grid::Grid(const Grid&) {
    NOTIMP;
}


Grid::Projection Grid::projection() const {
    NOTIMP;
}


UnstructuredGrid::UnstructuredGrid(points_t&& points) : points_(points) {
    NOTIMP;
}


}  // namespace atlas
