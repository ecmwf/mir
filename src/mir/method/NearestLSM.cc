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

#include "eckit/log/Log.h"
#include "eckit/log/Timer.h"
#include "mir/config/LibMir.h"
#include "mir/lsm/LandSeaMasks.h"
#include "mir/param/RuntimeParametrisation.h"
#include "mir/repres/Representation.h"
#include "mir/util/Compare.h"
#include "mir/util/MIRGrid.h"
#include "mir/util/PointSearch.h"


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


void NearestLSM::assemble(util::MIRStatistics&, WeightMatrix& W, const repres::Representation& rin, const repres::Representation& rout) const {

    util::MIRGrid in(rin.grid());
    util::MIRGrid out(rout.grid());


    eckit::Log::debug<LibMir>() << "NearestLSM::assemble (input: " << rin << ", output: " << rout << ")" << std::endl;
    eckit::TraceTimer<LibMir> timer("NearestLSM::assemble");


    // get the land-sea masks, with boolean masking on point (node) indices
    double here = timer.elapsed();

    const lsm::LandSeaMasks masks = getMasks(rin, rout);
    ASSERT(masks.active());

    eckit::Log::debug<LibMir>() << "NearestLSM compute LandSeaMasks " << timer.elapsed() - here << std::endl;


    // compute masked/not-masked search trees
    here = timer.elapsed();

    const std::vector< bool > &imask = masks.inputMask();
    const std::vector< bool > &omask = masks.outputMask();
    ASSERT(imask.size() == W.cols());
    ASSERT(omask.size() == W.rows());

    util::PointSearch sptree_masked    (rin, util::compare::IsMaskedFn   (imask));
    util::PointSearch sptree_notmasked (rin, util::compare::IsNotMaskedFn(imask));

    eckit::Log::debug<LibMir>() << "NearestLSM compute masked/not-masked search trees " << timer.elapsed() - here << std::endl;


    // search nearest neighbours matching in/output masks
    // - output mask (omask) operates on matrix row index (i)
    // - input mask (imask) operates on matrix column index (j)
    here = timer.elapsed();

    std::vector<WeightMatrix::Triplet> mat;
    mat.reserve(W.rows());

    const eckit::ScopedPtr<repres::Iterator> it(rout.iterator());
    size_t ip = 0;
    while (it->next()) {
        ASSERT(ip < W.rows());

        // pick the (input) search tree matching the output mask
        util::PointSearch& sptree(
            omask[ip] ? sptree_masked
            : sptree_notmasked );

        // perform nearest neighbour search
        // - p: output grid node to look neighbours for
        // - q: input grid node closest to p (accessed as qmeta.point())
        // - j: index of q in input grid (or input field)
        const util::PointSearch::PointType      p(it->point3D());
        const util::PointSearch::PointValueType qmeta = sptree.closestPoint(p);
        const size_t jp = qmeta.payload();

        // insert entry into uncompressed matrix structure
        mat.push_back(WeightMatrix::Triplet(ip, jp, 1.));

        ++ip;
    }
    eckit::Log::debug<LibMir>() << "NearestLSM search nearest neighbours matching in/output masks " << timer.elapsed() - here << std::endl;


    // fill-in sparse matrix
    here = timer.elapsed();
    W.setFromTriplets(mat);
    eckit::Log::debug<LibMir>() << "NearestLSM fill-in sparse matrix " << timer.elapsed() - here << std::endl;
}


lsm::LandSeaMasks NearestLSM::getMasks(const repres::Representation& in, const repres::Representation& out) const {
    param::RuntimeParametrisation runtime(parametrisation_);
    runtime.set("lsm", true); // Force use of LSM
    return lsm::LandSeaMasks::lookup(runtime, in, out);
}


void NearestLSM::applyMasks(WeightMatrix&, const lsm::LandSeaMasks&) const {
    // FIXME this function should not be overriding to do nothing
}


void NearestLSM::print(std::ostream &out) const {
    out << "NearestLSM[]";
}


namespace {
static MethodBuilder< NearestLSM > __method("nearest-lsm");
}


}  // namespace method
}  // namespace mir

