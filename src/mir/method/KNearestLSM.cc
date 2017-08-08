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

#include "eckit/utils/MD5.h"
#include "mir/lsm/LandSeaMasks.h"
#include "mir/param/RuntimeParametrisation.h"
#include "mir/method/distance/NearestNeighbourLSM.h"


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


void KNearestLSM::assemble(
        util::MIRStatistics& stats,
        WeightMatrix& W,
        const repres::Representation& in,
        const repres::Representation& out) const {

    // get distance weighting method
    const distance::NearestNeighbourLSM calculateWeights(parametrisation_, getMasks(in, out));

    // assemble with specific distance weighting method
    Nearest::assemble(stats, W, in, out, calculateWeights);
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


std::string KNearestLSM::distanceWeighting() const {

    // DistanceWeightingFactory cannot instantiate this method because it
    // requires knowledge of LandSeaMasks (from the interpolation method)
    NOTIMP;

    return "nearest-neighbour-lsm";
}


namespace {
static MethodBuilder< KNearestLSM > __method1("k-nearest-lsm");
static MethodBuilder< KNearestLSM > __method2("nearest-lsm");
}


}  // namespace method
}  // namespace mir

