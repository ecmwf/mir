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


#include "mir/method/KNearestLSM.h"

#include "eckit/log/BigNum.h"
#include "eckit/log/Log.h"
#include "eckit/log/Plural.h"
#include "eckit/log/ProgressTimer.h"
#include "eckit/log/TraceTimer.h"
#include "eckit/utils/MD5.h"
#include "mir/config/LibMir.h"
#include "mir/lsm/LandSeaMasks.h"
#include "mir/param/RuntimeParametrisation.h"
#include "mir/repres/Iterator.h"
#include "mir/repres/Representation.h"
#include "mir/util/PointSearch.h"


namespace mir {
namespace method {


KNearestLSM::KNearestLSM(const param::MIRParametrisation &param) :
    Nearest(param),
    nclosest_(4) {
    param.get("nclosest", nclosest_);
}


KNearestLSM::~KNearestLSM() {
}


void KNearestLSM::hash(eckit::MD5& md5) const {
    Nearest::hash(md5);
    md5 << nclosest_;
}


void KNearestLSM::assemble(util::MIRStatistics&,
                           WeightMatrix& W,
                           const repres::Representation& in,
                           const repres::Representation& out) const {

    eckit::Log::debug<LibMir>() << "KNearestLSM::assemble (input: " << in << ", output: " << out << ")" << std::endl;
    eckit::TraceTimer<LibMir> timer("KNearestLSM::assemble");


    // calculate/get the land-sea masks
    double here = timer.elapsed();
    const lsm::LandSeaMasks masks = getMasks(in, out);
    ASSERT(masks.active());

    const std::vector< bool > &imask = masks.inputMask();
    const std::vector< bool > &omask = masks.outputMask();
    ASSERT(imask.size() == W.cols());
    ASSERT(omask.size() == W.rows());

    eckit::Log::debug<LibMir>() << "KNearestLSM calculate LandSeaMasks " << timer.elapsed() - here << std::endl;


    // calculate k-d tree
    here = timer.elapsed();
    util::PointSearch sptree(parametrisation_, in);
    eckit::Log::debug<LibMir>() << "KNearestLSM calculate k-d tree " << timer.elapsed() - here << std::endl;


    // search nearest neighbours matching in/output masks
    // - ip: matrix row, or output point index, indexing output mask
    // - jp: matrix column, or input point index, indexing input mask
    here = timer.elapsed();

    const size_t out_npts = out.numberOfPoints();
    const size_t nclosest = this->nclosest();

    std::vector<util::PointSearch::PointValueType> closest;
    size_t nbDifferentTypes = 0;

    std::vector<WeightMatrix::Triplet> mat;
    mat.reserve(W.rows());

    {
        eckit::ProgressTimer progress("Locating", out_npts, "point", double(5), eckit::Log::debug<LibMir>());

        const eckit::ScopedPtr<repres::Iterator> it(out.iterator());
        size_t ip = 0;
        while (it->next()) {
            ASSERT(ip < W.rows());
            ++progress;

            const util::PointSearch::PointType p(it->point3D());
            sptree.closestNPoints(p, nclosest, closest);
            ASSERT(!closest.empty());

            bool foundSameType = false;
            size_t jp = 0;

            for (auto p : closest) {
                jp = p.payload();
                ASSERT(jp < imask.size());

                if (omask[ip] == imask[jp]) {
                    foundSameType = true;
                    break;
                }
            }

            if (!foundSameType) {
                jp = closest.front().payload();
                ++nbDifferentTypes;
            }

            // insert entry into uncompressed matrix structure
            mat.push_back(WeightMatrix::Triplet(ip, jp, 1.));

            ++ip;
        }
    }
    eckit::Log::debug<LibMir>() << "KNearestLSM search nearest neighbours (up to " << eckit::Plural(nclosest, "point") << ") matching type took " << timer.elapsed() - here << "s" << std::endl;
    eckit::Log::debug<LibMir>() << "KNearestLSM assigned weights to " << eckit::Plural(nbDifferentTypes, "point") << " of different type, out of " << eckit::BigNum(out_npts) << std::endl;


    // fill-in sparse matrix
    here = timer.elapsed();
    W.setFromTriplets(mat);
    eckit::Log::debug<LibMir>() << "KNearestLSM fill-in sparse matrix " << timer.elapsed() - here << std::endl;
}


void KNearestLSM::applyMasks(WeightMatrix&, const lsm::LandSeaMasks&) const {
    // FIXME this function should not be overriding to do nothing
}


lsm::LandSeaMasks KNearestLSM::getMasks(const repres::Representation& in, const repres::Representation& out) const {
    param::RuntimeParametrisation runtime(parametrisation_);
    runtime.set("lsm", true); // Force use of LSM
    return lsm::LandSeaMasks::lookup(runtime, in, out);
}


void KNearestLSM::print(std::ostream &out) const {
    out << "KNearestLSM[nclosest=" << nclosest_ << "]";
}


const char *KNearestLSM::name() const {
    return  "k-nearest-lsm";
}


size_t KNearestLSM::nclosest() const {
    return nclosest_;
}


namespace {
static MethodBuilder< KNearestLSM > __method("k-nearest-lsm");
}


}  // namespace method
}  // namespace mir

