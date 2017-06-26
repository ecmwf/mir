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

#include <vector>
#include "eckit/geometry/Point3.h"
#include "eckit/log/Log.h"
#include "atlas/array/ArrayView.h"
#include "atlas/array_fwd.h"
#include "atlas/grid.h"
#include "mir/config/LibMir.h"
#include "mir/param/MIRParametrisation.h"
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

    using eckit::geometry::LON;
    using eckit::geometry::LAT;

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


// Normalize weights triplets such that sum(weights) = 1
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


// Find nearest North-South bounding j indices
void StructuredMethod::boundNorthSouth(size_t& jNorth, size_t& jSouth, const double& lat, const std::vector<double>& latitudes) const {
    const size_t Nj = latitudes.size();
    ASSERT(Nj > 1);

    // locate latitude indices just North and South of given latitude
    std::vector<double>::const_reverse_iterator above = std::lower_bound(latitudes.rbegin(), latitudes.rend(), lat);


    jNorth = (Nj - static_cast<size_t>(std::distance(latitudes.rbegin(), above))) - 1;
    jSouth = jNorth + 1;

    ASSERT(0 <= jNorth && jNorth < Nj-1);
    ASSERT(0 < jSouth && jSouth <= Nj-1);
}


// Find nearest West-East bounding i indices
void StructuredMethod::boundWestEast(size_t& iWest, size_t& iEast, const double& lon, const size_t& Ni, const size_t& iStart) const {
    ASSERT(Ni > 1);

    // locate longitude indices just West and East of given longitude (in-row)
    iWest = static_cast<size_t>(std::floor(static_cast<double>(lon * Ni) / 360.));
    iEast = (iWest + 1) % Ni;
    ASSERT(iWest < Ni);

    // adjust starting indices to value indices (absolute)
    iWest += iStart;
    iEast += iStart;
}


void StructuredMethod::assemble(util::MIRStatistics&, WeightMatrix& W, const repres::Representation& rin, const repres::Representation& rout) const {
    util::MIRGrid in = rin.grid();

    eckit::Log::debug<LibMir>() << "StructuredMethod::assemble (input: " << rin << ", output: " << rout << ")..." << std::endl;

    // FIXME for the moment
    if (!in.domain().isGlobal()) {
        throw eckit::UserError("This interpolation method is only for global input grids.", Here());
    }
    if (!rout.domain().isGlobal()) {
        throw eckit::UserError("This interpolation method is only for global output grids.", Here());
    }

    atlas::grid::StructuredGrid gin(in);
    if (!gin) {
        throw eckit::UserError("This interpolation method is only for Structured grids as input.", Here());
    }

    assemble(W, gin, rout);
    eckit::Log::debug<LibMir>() << "StructuredMethod::assemble." << std::endl;
}


}  // namespace method
}  // namespace mir

