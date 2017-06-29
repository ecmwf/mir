/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @date Sep 2016


#include "mir/method/StructuredLinear.h"

#include "atlas/interpolation/element/Triag3D.h"
#include "atlas/interpolation/method/Ray.h"
#include "mir/repres/Iterator.h"
#include "mir/repres/Representation.h"
#include "mir/util/MIRGrid.h"


namespace mir {
namespace method {


namespace {
static MethodBuilder< StructuredLinear > __method("structured-linear");
}


StructuredLinear::StructuredLinear(const param::MIRParametrisation& param) :
    StructuredMethod(param) {
}


StructuredLinear::~StructuredLinear() {
}


void StructuredLinear::assembleStructuredInput(WeightMatrix& W, const repres::Representation& rin, const repres::Representation& rout) const {

    atlas::grid::StructuredGrid in(rin.grid());
    ASSERT(in);

    atlas::Grid out(rout.grid());
    ASSERT(out);


    /*
     * get from input grid:
     * - latitudes (assume they are sorted descending)
     * - accumulated pl array (to know global node indices)
     * - lon/lat points (TODO not necessary)
     */
    const std::vector<double>& latitudes = in.y();
    ASSERT(latitudes.front() > latitudes.back());
    ASSERT(latitudes.size() == in.nx().size());
    ASSERT(latitudes.size() >= 2);

    std::vector<size_t> pl_sum(in.nx().size() + 1, 0);
    std::partial_sum(in.nx().begin(), in.nx().end(), ++pl_sum.begin());
    ASSERT(static_cast<size_t>(pl_sum.back()) == in.size());


    // get from input grid lon/lat points
    std::vector< atlas::PointXY > in_points;
    in_points.reserve(in.size());
    for (const atlas::PointXY p: in.xy()) {
        in_points.push_back(p);
    }


    // fill sparse matrix using triplets (reserve assuming all-triangles interpolations)
    triplets_t triplets;
    triplets.reserve(3 * out.size());


    // interpolate each output point in turn
    eckit::ScopedPtr<repres::Iterator> it(rout.unrotatedIterator());
    Latitude lat;
    Longitude lon;
    size_t i = 0;

    while (it->next(lat, lon)) {
        ASSERT(i < out.size());

        triplets_t trip;

//        if (inDomain.contains(lat, lon)*/) {
//
//            ++i;
//            continue;
//
//        }

        if (lat > latitudes.front()) {

            // interpolate on above-North latitudes
            const size_t Ni = size_t(in.nx().front());
            const size_t iStart = 0;

            size_t l[2];
            boundWestEast(l[0], l[1], lon.value(), Ni, iStart);

            trip = { WeightMatrix::Triplet(i, l[0], in_points[l[1]].x() - lon.value()),
                     WeightMatrix::Triplet(i, l[1], lon.value() - in_points[l[0]].x()) };

        } else if (lat < latitudes.back()) {

            // interpolate on below-South latitudes
            // - pl_sum.rbegin() (last position) is total number of points sum(j=0; j=Nj, pl[j]),
            // - pl_sum.rbegin()[1] (before-last position) is sum(j=0; j=Nj-1, pl[j])
            const size_t Ni = size_t(in.nx().back());
            const size_t iStart = pl_sum.rbegin()[1];

            size_t l[2];
            boundWestEast(l[0], l[1], lon.value(), Ni, iStart);

            trip = { WeightMatrix::Triplet(i, l[0], in_points[l[1]].x() - lon.value() ),
                     WeightMatrix::Triplet(i, l[1], lon.value() - in_points[l[0]].x() ) };

        } else {

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
            boundNorthSouth(j_north, j_south, lat.value(), latitudes);

            size_t q[4];
            boundWestEast(q[0], q[1], lon.value(), in.nx(j_north), pl_sum[j_north]);
            boundWestEast(q[2], q[3], lon.value(), in.nx(j_south), pl_sum[j_south]);

            // convert working longitude/latitude coordinates to 3D
            point_3d_t ip;
            point_3d_t qp[4];
            eckit::geometry::lonlat_to_3d(lon.value(), lat.value(), ip.data());
            for (size_t k = 0; k < 4; ++k) {
                eckit::geometry::lonlat_to_3d(in_points[q[k]].data(), qp[k].data());
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
            const double dist2_q0_q3 = point_3d_t::distance2(qp[3], qp[0]);
            const double dist2_q1_q2 = point_3d_t::distance2(qp[2], qp[1]);

            const double edgeEpsilon = 1.e-11 * std::min(dist2_q0_q3, dist2_q1_q2);
            ASSERT(edgeEpsilon >= 0);

            // project on first triangle (0 or 2) depending on split, if that fails try next (1 or 3)
            const size_t T[4][3] = {{1,0,2}, {2,3,1}, {0,2,3}, {3,1,0}};
            size_t w = eckit::types::is_strictly_greater(dist2_q0_q3, dist2_q1_q2)? 0 : 2;

            using atlas::interpolation::element::Triag3D;
            using atlas::interpolation::method::Intersect;
            using atlas::interpolation::method::Ray;

            Intersect inter;
            Triag3D tri = Triag3D(qp[T[w][0]].data(), qp[T[w][1]].data(), qp[T[w][2]].data());
            if (!(inter = tri.intersects(Ray(ip.data()), edgeEpsilon))) {
                ++w;
                tri = Triag3D(qp[T[w][0]].data(), qp[T[w][1]].data(), qp[T[w][2]].data());
                if (!(inter = tri.intersects(Ray(ip.data()), edgeEpsilon))) {
                    throw eckit::UserError("Cannot determine an intersecting triangle (consider", Here());
                }
            }
            ASSERT(inter);

            // weights are the linear Lagrange function evaluated at u,v (aka barycentric coordinates)
            trip = { WeightMatrix::Triplet(i, q[T[w][0]], 1. - inter.u - inter.v),
                     WeightMatrix::Triplet(i, q[T[w][1]], inter.u),
                     WeightMatrix::Triplet(i, q[T[w][2]], inter.v) };
        }


        // insert local point weights (normalized) into matrix "filler"
        normalise(trip);
        std::copy(trip.begin(), trip.end(), std::back_inserter(triplets));

        ++i;
    }

    // fill sparse matrix
    W.setFromTriplets(triplets);
}


const char* StructuredLinear::name() const {
    return  "structured-linear";
}


void StructuredLinear::hash(eckit::MD5& md5) const {
    StructuredMethod::hash(md5);
}


void StructuredLinear::print(std::ostream& out) const {
    out << "StructuredLinear[]";
}


}  // namespace method
}  // namespace mir

