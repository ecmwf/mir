/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @date June 2017


#include "mir/method/StructuredMethod.h"

#include "eckit/geometry/Point3.h"
#include "eckit/log/Log.h"
#include "atlas/array/ArrayView.h"
#include "atlas/array_fwd.h"
#include "atlas/grid.h"
#include "mir/config/LibMir.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Iterator.h"
#include "mir/repres/Representation.h"
#include "mir/util/Compare.h"
#include "mir/util/MIRGrid.h"


namespace mir {
namespace method {


StructuredMethod::StructuredMethod(const param::MIRParametrisation& param) :
    MethodWeighted(param) {
}


StructuredMethod::~StructuredMethod() {
}


void StructuredMethod::left_right_lon_indexes(
        Longitude& in,
        const std::vector<point_ll_t>& coords,
        const size_t start,
        const size_t end,
        size_t& left,
        size_t& right ) const {

    right = start; // take the first if there's a wrap
    left  = start;

    Longitude right_lon = 360.;
//    Longitude left_lon  =   0.;
    for (size_t i = start; i < end; ++i) {

        const Longitude& val = coords[i].second;
        ASSERT((0. <= val) && (val <= 360.));

        if (val <= in) {
//            left_lon = val;
            left     = i;
        } else if (val < right_lon) {
            right_lon = val;
            right     = i;
        }

    }

    ASSERT(left  >= start);
    ASSERT(right >= start);
    ASSERT(right != left);
    ASSERT(coords[left].first == coords[right].first);
}


void StructuredMethod::normalise(triplets_t& triplets) const {
    ASSERT(triplets.size());

    // sum all calculated weights for normalisation
    double sum = 0.;
    for (const eckit::linalg::Triplet& t: triplets) {
        sum += t.value();
    }

    // now normalise all weights according to the total
    const double invSum = 1.0 / sum;
    for (eckit::linalg::Triplet& t: triplets) {
        t.value() *= invSum;
    }
}


void StructuredMethod::getRepresentationPoints(const repres::Representation& r, std::vector<point_ll_t> &points, Latitude& minimum, Latitude& maximum) const {
    const size_t N = r.grid().size();
    points.resize(N);
    minimum = 0;
    maximum = 0;

    eckit::ScopedPtr<repres::Iterator> it(r.unrotatedIterator());
    Latitude lat;
    Longitude lon;
    size_t i = 0;

    while (it->next(lat, lon)) {
        if (!i || lat < minimum) minimum = lat;
        if (!i || lat > maximum) maximum = lat;
        ASSERT(i < N);
        points[i++] = point_ll_t(lat, lon);
    }

    ASSERT(minimum < maximum);
}


void StructuredMethod::getRepresentationLatitudes(const repres::Representation& r, std::vector<Latitude>& latitudes) const {
    atlas::grid::StructuredGrid in(r.grid());
    ASSERT(in);

    const std::vector<long>& pl = in.nx();
    ASSERT(pl.size() >= 2);

    latitudes.clear();
    latitudes.reserve(pl.size());

    eckit::ScopedPtr<repres::Iterator> it(r.unrotatedIterator());
    Latitude lat;
    Longitude lon;
    for (long Nj: pl) {
        ASSERT(Nj >= 2);
        for (long i = 0; i < Nj; ++i) {
            ASSERT(it->next(lat, lon));
            if (i == 0) {
                latitudes.push_back(lat);
            }
        }
    }

    ASSERT(!it->next(lat, lon));
}

void StructuredMethod::boundNorthSouth(const Latitude& lat, const std::vector<Latitude>& latitudes, size_t& jNorth, size_t& jSouth) const {
    const size_t Nj = latitudes.size();
    ASSERT(Nj > 1);

    // locate latitude indices just North and South of given latitude
    std::vector<Latitude>::const_reverse_iterator above = std::lower_bound(latitudes.rbegin(), latitudes.rend(), lat);

    jNorth = (Nj - static_cast<size_t>(std::distance(latitudes.rbegin(), above))) - 1;
    jSouth = jNorth + 1;

    ASSERT(0 <= jNorth && jNorth < Nj-1);
    ASSERT(0 < jSouth && jSouth <= Nj-1);
}


void StructuredMethod::boundWestEast(const Longitude& lon, const size_t& Ni, const size_t& iStart, size_t& iWest, size_t& iEast) const {
    ASSERT(Ni > 1);

    // locate longitude indices just West and East of given longitude (in-row)
    iWest = size_t(Longitude((lon * Ni) / 360.).value());
    iEast = (iWest + 1) % Ni;
    ASSERT(iWest < Ni);

    // adjust starting indices to value indices (absolute)
    iWest += iStart;
    iEast += iStart;
}


void StructuredMethod::assemble(util::MIRStatistics&, WeightMatrix& W, const repres::Representation& rin, const repres::Representation& rout) const {
    eckit::Log::debug<LibMir>() << "StructuredMethod::assemble (input: " << rin << ", output: " << rout << ")..." << std::endl;

    // FIXME for the moment
    if (!rin.domain().isGlobal()) {
        throw eckit::UserError("This interpolation method is only for global input grids.", Here());
    }
    if (!rout.domain().isGlobal()) {
        throw eckit::UserError("This interpolation method is only for global output grids.", Here());
    }

    // ensure grid is structured and non-rotated
    atlas::Grid in = rin.grid();
    if (!atlas::grid::StructuredGrid(in) || in.projection()) {
        throw eckit::UserError("This interpolation method is only for non-rotated structured grids as input.", Here());
    }

    assembleStructuredInput(W, rin, rout);
    eckit::Log::debug<LibMir>() << "StructuredMethod::assemble." << std::endl;
}


}  // namespace method
}  // namespace mir

