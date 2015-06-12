/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Tiago Quintino
/// @author Pedro Maciel
/// @date May 2015


#include "mir/method/Nearest.h"

#include <string>
#include <limits>
#include <vector>

#include "mir/util/PointSearch.h"
#include "mir/param/MIRParametrisation.h"

#include "atlas/actions/BuildXYZField.h"

namespace mir {
namespace method {


Nearest::Nearest(const param::MIRParametrisation &param) :
    MethodWeighted(param),
    epsilon_(std::numeric_limits<double>::epsilon()) {

    param.get("epsilon", epsilon_);

}


Nearest::~Nearest() {
}


const char *Nearest::name() const {
    return  "k-nearest";
}


void Nearest::hash(eckit::MD5 &md5) const {
    MethodWeighted::hash(md5);
    md5 << epsilon_;
}


void Nearest::assemble(WeightMatrix &W, const atlas::Grid &in, const atlas::Grid &out) const {

    size_t nclosest = this->nclosest();

    util::PointSearch sptree(in.mesh());

    const atlas::Mesh &o_mesh = out.mesh();

    // output points
    atlas::FunctionSpace &o_nodes = o_mesh.function_space("nodes");
    atlas::actions::build_xyz_field(o_nodes,"xyz");
    atlas::ArrayView<double, 2> ocoords(o_nodes.field("xyz"));

    const size_t out_npts = o_nodes.shape(0);

    // init structure used to fill in sparse matrix
    std::vector<WeightMatrix::Triplet > weights_triplets;
    weights_triplets.reserve(out_npts * nclosest);

    std::vector<atlas::PointIndex3::Value> closest;

    std::vector<double> weights;
    weights.reserve(nclosest);

    for (size_t ip = 0; ip < out_npts; ++ip) {

        // get the reference output point
        eckit::geometry::Point3 p(ocoords[ip].data());

        // find the closest input points to this output
        sptree.closestNPoints(p, nclosest, closest);

        const size_t npts = closest.size();

        // then calculate the nearest neighbour weights
        weights.resize(npts, 0.0);

        // sum all calculated weights for normalisation
        double sum = 0.0;

        for (size_t j = 0; j < npts; ++j) {
            // one of the closest points
            eckit::geometry::Point3 np = closest[j].point();

            // calculate distance squared and weight
            const double d2 = eckit::geometry::Point3::distance2(p, np);
            weights[j] = 1. / (1. + d2);

            // also work out the total
            sum += weights[j];
        }

        ASSERT(sum > 0.0);

        // now normalise all weights according to the total
        for (size_t j = 0; j < npts; j++) {
            weights[j] /= sum;
        }

        // insert the interpolant weights into the global (sparse) interpolant matrix
        for (int i = 0; i < npts; ++i) {
            size_t index = closest[i].payload();
            weights_triplets.push_back(WeightMatrix::Triplet(ip, index, weights[i]));
        }
    }

    // fill-in sparse matrix
    W.setFromTriplets(weights_triplets.begin(), weights_triplets.end());
}


}  // namespace method
}  // namespace mir

