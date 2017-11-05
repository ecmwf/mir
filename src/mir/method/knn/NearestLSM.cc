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


#include "mir/method/knn/NearestLSM.h"

#include "eckit/utils/MD5.h"
#include "mir/lsm/LandSeaMasks.h"
#include "mir/param/RuntimeParametrisation.h"
#include "mir/method/knn/distance/NearestLSMWithLowestIndex.h"


namespace mir {
namespace method {
namespace knn {


NearestLSM::NearestLSM(const param::MIRParametrisation& param) : KNearestNeighbours(param) {
}


NearestLSM::~NearestLSM() {
}


void NearestLSM::assemble(
        util::MIRStatistics& stats,
        WeightMatrix& W,
        const repres::Representation& in,
        const repres::Representation& out) const {

    // get distance weighting method
    const distance::NearestLSMWithLowestIndex calculateWeights(parametrisation_, getMasks(in, out));

    // assemble with specific distance weighting method
    KNearestNeighbours::assemble(stats, W, in, out, calculateWeights);
}


void NearestLSM::applyMasks(WeightMatrix&, const lsm::LandSeaMasks&) const {
    // FIXME this function should not be overriding to do nothing
}


lsm::LandSeaMasks NearestLSM::getMasks(const repres::Representation& in, const repres::Representation& out) const {
    param::RuntimeParametrisation runtime(parametrisation_);

    // Force use of LSM (unless it is already set)
    runtime.set("lsm", true);

    std::string select;
    if (!parametrisation_.get("lsm-selection", select) || select == "none") {
        runtime.set("lsm-selection", "named");
        runtime.set("lsm-named", "1km");
    }
    if (parametrisation_.get("lsm-selection-input", select) && select == "none") {
        runtime.hide("lsm-selection-input");
    }
    if (parametrisation_.get("lsm-selection-output", select) && select == "none") {
        runtime.hide("lsm-selection-output");
    }

    lsm::LandSeaMasks masks = lsm::LandSeaMasks::lookup(runtime, in, out);
    ASSERT(masks.active());
    return masks;
}


const char* NearestLSM::name() const {
    return "nearest-lsm";
}


std::string NearestLSM::distanceWeighting() const {
    return "nearest-lsm-with-lowest-index";
}


namespace {
static MethodBuilder< NearestLSM > __method("nearest-lsm");
}


}  // namespace knn
}  // namespace method
}  // namespace mir

