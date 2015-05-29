/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Tiago Quintino
/// @author Pedro Maciel
/// @date May 2015


#include "mir/method/NearestLSM.h"

#if 0
#include "eckit/log/BigNum.h"
#include "eckit/log/Plural.h"
#include "mir/data/MIRField.h"
#endif
#include "eckit/log/Log.h"
#include "eckit/log/Timer.h"
#include "mir/lsm/LandSeaMasks.h"
#include "mir/param/RuntimeParametrisation.h"
#include "mir/util/Compare.h"
#include "mir/util/PointSearch.h"


using eckit::Log;


namespace mir {
namespace method {


NearestLSM::NearestLSM(const param::MIRParametrisation &param) :
    MethodWeighted(param) {
}


NearestLSM::~NearestLSM() {
}


const char *NearestLSM::name() const {
    return  "nearest-lsm";
}


void NearestLSM::assemble(WeightMatrix &W, const atlas::Grid &in, const atlas::Grid &out) const {


    const std::string name("MethodWeighted::getMatrix");
    eckit::Timer timer(name);


    // get the land-sea masks, with boolean masking on point (node) indices
    double here = timer.elapsed();

    const lsm::LandSeaMasks masks = getMasks(in, out);
    ASSERT(masks.active());
    Log::info() << name << " compute LandSeaMasks " << timer.elapsed() - here << std::endl;


    // compute masked/not-masked search trees
    here = timer.elapsed();

    const std::vector< bool > &imask = masks.inputMask();
    const std::vector< bool > &omask = masks.outputMask();


    ASSERT(imask.size() == W.cols());
    ASSERT(omask.size() == W.rows());

    const util::compare::is_masked_fn     is_imasked    (imask);
    const util::compare::is_not_masked_fn is_inotmasked (imask);

    util::PointSearch sptree_masked    (in.mesh(), is_imasked);
    util::PointSearch sptree_notmasked (in.mesh(), is_inotmasked);

    Log::info() << name << " compute masked/not-masked search trees " << timer.elapsed() - here << std::endl;


    // compute the output nodes coordinates
    here = timer.elapsed();

    const atlas::Mesh &o_mesh = out.mesh();
    ASSERT(o_mesh.has_function_space("nodes"));
    ASSERT(o_mesh.function_space("nodes").has_field("xyz"));
    atlas::ArrayView< double, 2 > ocoords(
                o_mesh.function_space("nodes").field("xyz") );

    Log::info() << name << " compute the output nodes coordinates " << timer.elapsed() - here << std::endl;


    // init sparse matrix uncompressed structure
    here = timer.elapsed();
    std::vector< Eigen::Triplet< double > > weights_triplets;
    weights_triplets.reserve(W.rows());
    Log::info() << name << " init sparse matrix uncompressed structure " << timer.elapsed() - here << std::endl;


    // search nearest neighbours matching in/output masks
    // - output mask (omask) operates on matrix row index (i)
    // - input mask (imask) operates on matrix column index (j)
    here = timer.elapsed();

    for (size_t i=0; i<W.rows(); ++i) {

        // pick the (input) search tree matching the output mask
        util::PointSearch& sptree(
                    omask[i]? sptree_masked
                            : sptree_notmasked );

        // perform nearest neighbour search
        // - p: output grid node to look neighbours for
        // - q: input grid node closest to p
        // - j: index of q in input grid (or input field)
        util::PointSearch::PointType      p(ocoords[i].data());
        util::PointSearch::PointValueType q = sptree.closestPoint(p);
        const size_t j = q.payload();

        // insert entry into uncompressed matrix structure
        weights_triplets.push_back(Eigen::Triplet< double >( i, j, 1.));

    }

    Log::info() << name << " search nearest neighbours matching in/output masks " << timer.elapsed() - here << std::endl;


    // fill-in sparse matrix
    here = timer.elapsed();
    W.setFromTriplets(weights_triplets.begin(), weights_triplets.end());
    Log::info() << name << " fill-in sparse matrix " << timer.elapsed() - here << std::endl;
}


lsm::LandSeaMasks NearestLSM::getMasks(const atlas::Grid &in, const atlas::Grid &out) const {
    param::RuntimeParametrisation runtime(parametrisation_);
    runtime.set("lsm", true); // Force use of LSM
    return lsm::LandSeaMasks::lookup(runtime, in, out);
}



WeightMatrix NearestLSM::applyMissingValues(const WeightMatrix &W, data::MIRField &field, size_t which) const {
    // This is possible, you can have missing values and use NearestLSM
    return MethodWeighted::applyMissingValues(W, field, which);
}

void NearestLSM::applyMasks(WeightMatrix &W, const lsm::LandSeaMasks &) const {
    // FIXME this function should not be overriding to do nothing
}

void NearestLSM::print(std::ostream &out) const {
    out << name() << "[]";
}


namespace {
static MethodBuilder< NearestLSM > __method("nearest-lsm");
}


}  // namespace method
}  // namespace mir

