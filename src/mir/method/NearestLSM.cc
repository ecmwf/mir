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

#include <algorithm>
#include <string>

#include "atlas/grids/ReducedGaussianGrid.h"
#include "eckit/log/BigNum.h"
#include "eckit/log/Log.h"
#include "eckit/log/Plural.h"
#include "eckit/log/Timer.h"
#include "eckit/utils/MD5.h"
#include "mir/data/MIRField.h"
#include "mir/lsm/LandSeaMasks.h"
#include "mir/lsm/LandSeaMasks.h"
#include "mir/param/RuntimeParametrisation.h"
#include "mir/util/Compare.h"


using eckit::Log;


namespace mir {
namespace method {


NearestLSM::NearestLSM(const param::MIRParametrisation &param) :
    Nearest(param) {
}


NearestLSM::~NearestLSM() {
}

lsm::LandSeaMasks NearestLSM::getMasks(const atlas::Grid &in, const atlas::Grid &out) const {
    param::RuntimeParametrisation runtime(parametrisation_);
    runtime.set("lsm", true); // Force use of LSM
    return lsm::LandSeaMasks::lookup(runtime, in, out);
}

const char *NearestLSM::name() const {
    return  "nearest-lsm";
}

size_t NearestLSM::nclosest() const {
    // TODO make this a real parameter and find a better default
    return 16;
}

void NearestLSM::hash( eckit::MD5& md5) const {
    MethodWeighted::hash(md5);
}

void NearestLSM::assemble(WeightMatrix &W, const atlas::Grid &in, const atlas::Grid &out) const {
    Nearest::assemble(W, in, out);
}

void NearestLSM::applyMasks(WeightMatrix& W, const lsm::LandSeaMasks& masks) const {
    eckit::Timer timer("NearestLSM::applyMasks");
    using mir::util::compare::is_approx_zero;

    Log::info() << "======== NearestLSM::applyMasks(" << masks << ")" << std::endl;
    ASSERT(masks.active());
    ASSERT(!masks.inputField().hasMissing());
    ASSERT(!masks.outputField().hasMissing());
    ASSERT(masks.inputField().dimensions() == 1);
    ASSERT(masks.inputField().dimensions() == 1);


    // build boolean masks (to isolate algorithm from the logical mask condition)
    const util::compare::is_greater_equal_fn< double > check_lsm(0.5);
    const std::vector< bool > imask = computeFieldMask(check_lsm, masks.inputField(),  0);
    const std::vector< bool > omask = computeFieldMask(check_lsm, masks.outputField(), 0);
    ASSERT(imask.size() == W.cols());
    ASSERT(omask.size() == W.rows());


    // apply corrections on inequality != (XOR) of logical masks,
    // then redistribute weights
    // - output mask (omask) operates on matrix row index, here i
    // - input mask (imask) operates on matrix column index, here j.col()
    // FIXME: hardcoded to *= 0.2
    size_t fix = 0;
    for (size_t i = 0; i < W.rows(); i++) {

        ASSERT(i < omask.size());

        // correct weight of non-matching input point weight contribution
        double sum = 0.;
        bool row_changed = false;
        for (WeightMatrix::InnerIterator j(W, i); j; ++j) {

            ASSERT(j.col() < imask.size());

            if (omask[i] != imask[j.col()]) {
                j.valueRef() *= 0.2;
                row_changed = true;
            }
            sum += j.value();
        }

        // apply linear redistribution if necessary
        if (row_changed && !is_approx_zero(sum)) {
            ++fix;
            for (WeightMatrix::InnerIterator j(W, i); j; ++j)
                j.valueRef() /= sum;
        }

    }


    // log corrections
    Log::info() << "NearestLSM: applyMasks corrected " << eckit::BigNum(fix) << " out of " << eckit::Plural(W.rows() ,"row") << std::endl;
}


void NearestLSM::print(std::ostream &out) const {
    out << "NearestLSM[]";
}


namespace {
static MethodBuilder< NearestLSM > __method("nearest-lsm");
}


}  // namespace method
}  // namespace mir

