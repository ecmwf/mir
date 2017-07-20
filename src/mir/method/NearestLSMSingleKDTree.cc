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


#include "mir/method/NearestLSMSingleKDTree.h"

//#include <forward_list>
//#include <utility>
#include "eckit/log/Log.h"
#include "eckit/log/BigNum.h"
#include "eckit/log/ETA.h"
#include "eckit/log/Plural.h"
#include "eckit/log/Timer.h"
#include "mir/config/LibMir.h"
#include "mir/lsm/LandSeaMasks.h"
#include "mir/param/RuntimeParametrisation.h"
#include "mir/repres/Iterator.h"
#include "mir/repres/Representation.h"
//#include "mir/util/Compare.h"
#include "mir/util/PointSearch.h"


namespace mir {
namespace method {


NearestLSMSingleKDTree::NearestLSMSingleKDTree(const param::MIRParametrisation &param) :
    MethodWeighted(param) {
}


NearestLSMSingleKDTree::~NearestLSMSingleKDTree() {
}


const char *NearestLSMSingleKDTree::name() const {
    return  "nearest-lsm";
}


void NearestLSMSingleKDTree::assemble(util::MIRStatistics&,
                          WeightMatrix& W,
                          const repres::Representation& in,
                          const repres::Representation& out) const {


    eckit::Log::debug<LibMir>() << "NearestLSMSingleKDTree::assemble (input: " << in << ", output: " << out << ")" << std::endl;
    eckit::TraceTimer<LibMir> timer("NearestLSMSingleKDTree::assemble");


    // get the land-sea masks, with boolean masking on point (node) indices
    double here = timer.elapsed();

    const lsm::LandSeaMasks masks = getMasks(in, out);
    ASSERT(masks.active());

    eckit::Log::debug<LibMir>() << "NearestLSMSingleKDTree compute LandSeaMasks " << timer.elapsed() - here << std::endl;


    // TODO take from parametrisation, plenty of examples out there
    const size_t nclosest = 4;
    std::vector<util::PointSearch::PointValueType> closest;


//    typedef std::pair< size_t, repres::Iterator::point_ll_t > different_type_t;
//    std::forward_list<different_type_t> different_types;
    size_t nbDifferentTypes = 0;

    const size_t out_npts = out.numberOfPoints();


    // compute masked/not-masked search trees
    here = timer.elapsed();

    const std::vector< bool > &imask = masks.inputMask();
    const std::vector< bool > &omask = masks.outputMask();
    ASSERT(imask.size() == W.cols());
    ASSERT(omask.size() == W.rows());

    util::PointSearch sptree(parametrisation_, in);

    eckit::Log::debug<LibMir>() << "NearestLSMSingleKDTree compute search tree " << timer.elapsed() - here << std::endl;


    // search nearest neighbours matching in/output masks
    // - output mask (omask) operates on matrix row index (i)
    // - input mask (imask) operates on matrix column index (j)
    here = timer.elapsed();

    std::vector<WeightMatrix::Triplet> mat;
    mat.reserve(W.rows());

    {
        eckit::Log::debug<LibMir>() << "Locating same-type neighbours (up to " << nclosest << ") for " << eckit::Plural(out_npts, "output point") << " from input " << in << std::endl;
        eckit::TraceTimer<LibMir> timerLocating("Locating");

        // output points
        const eckit::ScopedPtr<repres::Iterator> it(out.iterator());
        size_t ip = 0;
        while (it->next()) {
            ASSERT(ip < W.rows());

            if (ip && (ip % 10000 == 0)) {
                double rate = ip / timerLocating.elapsed();
                eckit::Log::debug<LibMir>()
                        << eckit::BigNum(ip) << " ..."  << eckit::Seconds(timerLocating.elapsed())
                        << ", rate: " << rate << " points/s, ETA: "
                        << eckit::ETA( (out_npts - ip) / rate )
                        << std::endl;
            }

            // perform nearest neighbour search
            // - p: output grid node to look neighbours for
            // - ip: matrix row, or output point index
            // - jp: matrix column, or input point index
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
//                different_types.push_front(different_type_t(ip, it->pointUnrotated()));
                ++nbDifferentTypes;
            }

            // insert entry into uncompressed matrix structure
            mat.push_back(WeightMatrix::Triplet(ip, jp, 1.));

            ++ip;
        }
    }
    eckit::Log::debug<LibMir>() << "NearestLSMSingleKDTree search nearest neighbours matching type " << timer.elapsed() - here << std::endl;


    eckit::Log::debug<LibMir>() << "Assigned weights to " << eckit::Plural(nbDifferentTypes, "point") << " of different type, out of " << eckit::BigNum(out_npts) << std::endl;
//    if (nbDifferentTypes) {
//        std::stringstream msg;
//        msg << "Assigned weights to " << eckit::Plural(nbDifferentTypes, "point") << " of different type";
//        eckit::Log::debug<LibMir>() << msg.str() << ":";
//        size_t count = 0;
//        for (const different_type_t& f : different_types) {
//            eckit::Log::debug<LibMir>() << "\n\tpoint " << f.first << " (lon, lat) = (" << f.second.lon.value() << ", " << f.second.lat.value() << ")";
//            if (++count > 10) {
//                eckit::Log::debug<LibMir>() << "\n\t...";
//                break;
//            }
//        }
//        eckit::Log::debug<LibMir>() << std::endl;
//    }


    // fill-in sparse matrix
    here = timer.elapsed();
    W.setFromTriplets(mat);
    eckit::Log::debug<LibMir>() << "NearestLSMSingleKDTree fill-in sparse matrix " << timer.elapsed() - here << std::endl;
}


lsm::LandSeaMasks NearestLSMSingleKDTree::getMasks(const repres::Representation& in, const repres::Representation& out) const {
    param::RuntimeParametrisation runtime(parametrisation_);
    runtime.set("lsm", true); // Force use of LSM
    return lsm::LandSeaMasks::lookup(runtime, in, out);
}


void NearestLSMSingleKDTree::applyMasks(WeightMatrix&, const lsm::LandSeaMasks&) const {
    // FIXME this function should not be overriding to do nothing
}


void NearestLSMSingleKDTree::print(std::ostream &out) const {
    out << "NearestLSMSingleKDTree[]";
}


namespace {
static MethodBuilder< NearestLSMSingleKDTree > __method("nearest-lsm-single-k-d-tree");
}


}  // namespace method
}  // namespace mir

