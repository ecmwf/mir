/*
 * (C) Copyright 1996- ECMWF.
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


#include "mir/method/knn/NearestLSM.h"

#include <memory>

#include "eckit/utils/MD5.h"

#include "mir/lsm/LandSeaMasks.h"
#include "mir/method/knn/distance/DistanceWeighting.h"
#include "mir/method/knn/pick/PickWithLSM.h"
#include "mir/param/RuntimeParametrisation.h"


namespace mir {
namespace method {
namespace knn {


NearestLSM::NearestLSM(const param::MIRParametrisation& param) : KNearestNeighbours(param) {
    param.get("nearest-method-with-lsm", nearestMethodWithLSM_ = "closest-lsm-with-lowest-index");
}


NearestLSM::~NearestLSM() = default;


void NearestLSM::assemble(util::MIRStatistics& stats, WeightMatrix& W, const repres::Representation& in,
                          const repres::Representation& out) const {

    // assemble with run-time neighbour-picking method
    std::unique_ptr<const pick::PickWithLSM> pick(
        pick::PickWithLSMFactory::build(nearestMethodWithLSM_, parametrisation_, getMasks(in, out)));

    ASSERT(pick);
    assembleCustomised(stats, W, in, out, *pick, distanceWeighting());
}


void NearestLSM::applyMasks(WeightMatrix&, const lsm::LandSeaMasks&) const {
    // FIXME this function should not be overriding to do nothing
}

static void setParametrisation(const param::MIRParametrisation&, param::RuntimeParametrisation& runtime) {

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


void NearestLSM::print(std::ostream& out) const {
    out << "NearestLSM[";
    MethodWeighted::print(out);
    out << ",nearestMethodWithLSM=" << nearestMethodWithLSM_ << ",distanceWeighting=" << distanceWeighting() << "]";
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
    auto o = dynamic_cast<const NearestLSM*>(&other);
    return o && nearestMethodWithLSM_ == o->nearestMethodWithLSM_ && KNearestNeighbours::sameAs(other) &&
           sameLsm(parametrisation_, o->parametrisation_);
}


const char* NearestLSM::name() const {
    return "nearest-lsm";
}


namespace {
static MethodBuilder<NearestLSM> __method("nearest-lsm");
}


}  // namespace knn
}  // namespace method
}  // namespace mir
