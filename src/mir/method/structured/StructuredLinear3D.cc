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


#include "mir/method/structured/StructuredLinear3D.h"

#include <memory>
#include <numeric>
#include <vector>

#include "eckit/types/FloatCompare.h"

#include "atlas/interpolation/element/Triag3D.h"
#include "atlas/interpolation/method/Ray.h"
#include "atlas/util/Earth.h"
#include "atlas/util/Point.h"

#include "mir/repres/Iterator.h"
#include "mir/repres/Representation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"
#include "mir/util/Trace.h"
#include "mir/util/Types.h"


namespace mir::method::structured {


static const MethodBuilder<StructuredLinear3D> __method("structured-linear-3d");


StructuredLinear3D::StructuredLinear3D(const param::MIRParametrisation& param) : StructuredMethod(param) {}


StructuredLinear3D::~StructuredLinear3D() = default;


bool StructuredLinear3D::sameAs(const Method& other) const {
    const auto* o = dynamic_cast<const StructuredLinear3D*>(&other);
    return (o != nullptr) && StructuredMethod::sameAs(other);
}


void StructuredLinear3D::assembleStructuredInput(WeightMatrix& W, const repres::Representation& in,
                                                 const repres::Representation& out) const {

    /*
     * get from input grid:
     * - latitudes (assume they are sorted descending)
     * - accumulated pl array (to know global node indices)
     *   - pl_sum.rbegin() (last position) is total number of points sum(j=0; j=Nj, pl[j]),
     *   - pl_sum.rbegin()[1] (before-last position) is sum(j=0; j=Nj-1, pl[j])
     */
    atlas::StructuredGrid gin(in.atlasGrid());
    ASSERT(gin);

    const auto& pl = gin.nx();
    ASSERT(pl.size());
    ASSERT(pl.front());
    ASSERT(pl.back());

    std::vector<Latitude> latitudes;
    getRepresentationLatitudes(in, latitudes);

    std::vector<size_t> pl_sum(pl.size() + 1, 0);
    std::partial_sum(pl.begin(), pl.end(), ++pl_sum.begin());
    ASSERT(static_cast<size_t>(pl_sum.back()) == in.numberOfPoints());

    // get input coordinates, checking min/max latitudes (Gaussian grids exclude the poles)
    std::vector<PointLatLon> icoords;
    Latitude min_lat;
    Latitude max_lat;
    getRepresentationPoints(in, icoords, min_lat, max_lat);
    Log::debug() << "StructureLinear::assemble latitude (min,max) = (" << min_lat << ", " << max_lat << ")"
                 << std::endl;

    // fill sparse matrix using triplets (reserve assuming all-triangle interpolations)
    triplet_vector_t triplets;
    size_t nbOutputPoints = out.numberOfPoints();
    triplets.reserve(3 * nbOutputPoints);

    // interpolate each output point in turn
    {
        trace::ProgressTimer progress("Interpolating", nbOutputPoints, {"point"});

        for (const std::unique_ptr<repres::Iterator> it(out.iterator()); it->next();) {
            ++progress;

            auto i = it->index();
            ASSERT(i < nbOutputPoints);

            triplet_vector_t trip;
            const auto& p = it->pointUnrotated();

            const bool too_much_north = p.lat() > max_lat;
            const bool too_much_south = p.lat() < min_lat;

            if (too_much_north || too_much_south) {
                ASSERT(too_much_north != too_much_south);

                auto Ni     = size_t(too_much_north ? pl.front() : pl.back());
                auto iStart = too_much_north ? 0 : pl_sum.rbegin()[1];

                size_t l[2];
                boundWestEast(p.lon(), Ni, iStart, l[0], l[1]);

                const Longitude& l0 = icoords[l[0]].lon();
                const Longitude& l1 = icoords[l[1]].lon();
                trip                = {WeightMatrix::Triplet(i, l[0], (l1 - p.lon()).value()),
                                       WeightMatrix::Triplet(i, l[1], (p.lon() - l0).value())};
            }
            else {

                /*
                 * setup quadrilateral: determine enveloping indices
                 * - first calculate number of division (local to the row)
                 * - then shift indices accurding to accumulated pl (global indices)
                 *
                 *   (q0)--------------(q1)  <- j_north
                 *    |                  \
                 *    |             (P)   \
                 *    |                    \
                 *    |                     \
                 *   (q2)------------------(q3) <- j_south
                 */

                size_t j_north;
                size_t j_south;
                boundNorthSouth(p.lat(), latitudes, j_north, j_south);

                size_t q[4];
                boundWestEast(p.lon(), size_t(pl[j_north]), pl_sum[j_north], q[0], q[1]);
                boundWestEast(p.lon(), size_t(pl[j_south]), pl_sum[j_south], q[2], q[3]);

                // convert working longitude/latitude coordinates to 3D
                atlas::PointXYZ qp[4];
                for (size_t k = 0; k < 4; ++k) {
                    const auto& ll = icoords[q[k]];

                    // notice the order
                    atlas::PointLonLat lonlat(ll.lon().value(), ll.lat().value());
                    qp[k] = atlas::util::Earth::convertSphericalToCartesian(atlas::to_pointlonlat(lonlat));
                }

                /*
                 * Triangle numbering and splitting reference (only one of the two pairs is chosen)
                 * These triangles were chosen so that the splitting quadrilateral cross-edge is the
                 * diagonal edge of interpolating reference triangle (for all four triangles).
                 * Starting triangle (0 or 2) us chosen using shortest cross-quadrilateral edge
                 * (that is, min( dist(q0,q3), dist(q1,q2) ))
                 *
                 *   (q0)--------------(q1)        (q0)--------------(q1)
                 *    |   T[0]      -    \          |   -    T[3]      \
                 *    |          -  (P)   \    or   |        -    (P)   \
                 *    |       -            \        |             -      \
                 *    |    -       T[1]     \       |    T[2]          -  \
                 *   (q2)------------------(q3)    (q2)------------------(q3)
                 */

                // pick an "edge epsilon" based on a characteristic length (shortest diagonal length)
                // (this scales linearly so it better compares with linear weights u,v,w)
                const double dist2_q0_q3 = Point3::distance2(qp[3], qp[0]);
                const double dist2_q1_q2 = Point3::distance2(qp[2], qp[1]);

                const double edgeEpsilon = 1.e-11 * std::min(dist2_q0_q3, dist2_q1_q2);
                ASSERT(edgeEpsilon >= 0);

                // project on first triangle (0 or 2) depending on split, if that fails try next (1 or 3)
                const size_t T[4][3] = {{1, 0, 2}, {2, 3, 1}, {0, 2, 3}, {3, 1, 0}};
                size_t w             = eckit::types::is_strictly_greater(dist2_q0_q3, dist2_q1_q2) ? 0 : 2;

                const Point3 p3d = it->point3D();
                const atlas::interpolation::method::Ray ray(p3d.data());
                atlas::interpolation::method::Intersect inter;

                using atlas::interpolation::element::Triag3D;
                Triag3D tri = Triag3D(qp[T[w][0]].data(), qp[T[w][1]].data(), qp[T[w][2]].data());
                if (!(inter = tri.intersects(ray, edgeEpsilon))) {
                    ++w;
                    tri = Triag3D(qp[T[w][0]].data(), qp[T[w][1]].data(), qp[T[w][2]].data());
                    if (!(inter = tri.intersects(ray, edgeEpsilon))) {
                        throw exception::UserError("Cannot determine an intersecting triangle (consider", Here());
                    }
                }
                ASSERT(inter);

                // weights are the linear Lagrange function evaluated at u,v (aka barycentric coordinates)
                trip = {WeightMatrix::Triplet(i, q[T[w][0]], 1. - inter.u - inter.v),
                        WeightMatrix::Triplet(i, q[T[w][1]], inter.u), WeightMatrix::Triplet(i, q[T[w][2]], inter.v)};
            }

            // insert local point weights (normalized) into matrix "filler"
            normalise(trip);
            std::copy(trip.begin(), trip.end(), std::back_inserter(triplets));
        }
    }

    // fill sparse matrix
    ASSERT_NONEMPTY_INTERPOLATION("StructuredLinear3D", !triplets.empty());
    W.setFromTriplets(triplets);
}


const char* StructuredLinear3D::name() const {
    return "structured-linear-3d";
}


void StructuredLinear3D::hash(eckit::MD5& md5) const {
    StructuredMethod::hash(md5);
}


void StructuredLinear3D::print(std::ostream& out) const {
    out << "StructuredLinear3D[";
    StructuredMethod::print(out);
    out << "]";
}


}  // namespace mir::method::structured
