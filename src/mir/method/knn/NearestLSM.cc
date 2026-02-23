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


#include "mir/method/knn/NearestLSM.h"

#include <memory>

#include "mir/lsm/LandSeaMasks.h"
#include "mir/method/knn/pick/Pick.h"
#include "mir/param/RuntimeParametrisation.h"
#include "mir/util/Exceptions.h"


namespace mir::method::knn {


NearestLSM::NearestLSM(const param::MIRParametrisation& param) : KNearestNeighbours(param), distanceWeighting_(param) {

    std::string nearestMethod = "nclosest";
    param.get("nearest-method", nearestMethod);
    pick_.reset(pick::PickFactory::build(nearestMethod, param));
}


void NearestLSM::assemble(util::MIRStatistics& stats, WeightMatrix& W, const repres::Representation& in,
                          const repres::Representation& out) const {

    // get distance weighting method
    std::unique_ptr<const distance::DistanceWeighting> method(
        distanceWeighting_.distanceWeighting(parametrisation_, getMasks(in, out)));
    ASSERT(method);

    // assemble with specific distance weighting method
    KNearestNeighbours::assemble(stats, W, in, out, *pick_, *method);
}


void NearestLSM::applyMasks(WeightMatrix& /*unused*/, const lsm::LandSeaMasks& /*unused*/) const {
    // FIXME this function should not be overriding to do nothing
}

static void setParametrisation(const param::MIRParametrisation& /*unused*/, param::RuntimeParametrisation& runtime) {

    // Force use of LSM (unless it is already set)
    runtime.set("lsm", true);
}

lsm::LandSeaMasks NearestLSM::getMasks(const repres::Representation& in, const repres::Representation& out) const {

    param::RuntimeParametrisation runtime(parametrisation_);
    setParametrisation(parametrisation_, runtime);

    lsm::LandSeaMasks masks = lsm::LandSeaMasks::lookup(runtime, in, out);
    ASSERT(masks.active());
    return masks;
}


static bool sameLsm(const param::MIRParametrisation& parametrisation1,
                    const param::MIRParametrisation& parametrisation2) {
    param::RuntimeParametrisation runtime1(parametrisation1);
    setParametrisation(parametrisation1, runtime1);

    param::RuntimeParametrisation runtime2(parametrisation2);
    setParametrisation(parametrisation2, runtime2);

    return lsm::LandSeaMasks::sameLandSeaMasks(runtime1, runtime2);
}


bool NearestLSM::sameAs(const Method& other) const {
    const auto* o = dynamic_cast<const NearestLSM*>(&other);
    return (o != nullptr) && KNearestNeighbours::sameAs(other) && sameLsm(parametrisation_, o->parametrisation_);
}


const pick::Pick& NearestLSM::pick() const {
    ASSERT(pick_);
    return *pick_;
}


const distance::DistanceWeighting& NearestLSM::distanceWeighting() const {
    return distanceWeighting_;
}


const char* NearestLSM::type() const {
    return "nearest-lsm";
}


static const MethodBuilder<NearestLSM> __method("nearest-lsm");


}  // namespace mir::method::knn
