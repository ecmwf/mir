/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Peter Bispham
/// @author Tiago Quintino
/// @author Pedro Maciel
/// @date May 2015


#include "mir/method/MethodWeighted.h"

#include <algorithm>
#include <map>
#include <string>
#include <sstream>

#include "atlas/Grid.h"
#include "eckit/log/Plural.h"
#include "eckit/log/Seconds.h"
#include "eckit/log/Timer.h"
#include "eckit/thread/AutoLock.h"
#include "eckit/thread/Mutex.h"

#include "mir/caching/WeightCache.h"
#include "mir/data/MIRField.h"
#include "mir/data/MIRFieldStats.h"
#include "mir/lsm/LandSeaMasks.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/util/Compare.h"


// using eckit::Log;
using mir::util::compare::is_approx_zero;
using mir::util::compare::is_approx_one;


namespace mir {
namespace method {


namespace {


static eckit::Mutex local_mutex;


static std::map<std::string, WeightMatrix> matrix_cache;


}  // (anonymous namespace)


MethodWeighted::MethodWeighted(const param::MIRParametrisation &parametrisation) :
    Method(parametrisation) {
    ASSERT(parametrisation.get("lsm.weight.adjustment", lsmWeightAdjustement_));
}


MethodWeighted::~MethodWeighted() {
}


// This returns a 'const' matrix so we ensure that we don't change it and break the in-memory cache
const WeightMatrix &MethodWeighted::getMatrix(const atlas::Grid &in, const atlas::Grid &out) const {

    eckit::Log::info() << "MethodWeighted::getMatrix " << *this << std::endl;

    eckit::Timer timer("MethodWeighted::getMatrix");

    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    double here = timer.elapsed();
    const lsm::LandSeaMasks masks = getMasks(in, out);
    eckit::Log::info() << "Compute LandSeaMasks " << timer.elapsed() - here << std::endl;

    eckit::Log::info() << "++++ LSM masks " << masks << std::endl;
    here = timer.elapsed();
    eckit::MD5 md5;
    md5 << *this;
    md5 << in;
    md5 << out;

    const eckit::MD5::digest_t md5_no_masks(md5.digest());
    md5 << masks;
    const eckit::MD5::digest_t md5_with_masks(md5.digest());
    eckit::Log::info() << "Compute md5 " << timer.elapsed() - here << std::endl;


    const std::string base_name      = std::string(name()) + "-" + in.shortName() + "-" + out.shortName();
    const std::string key_no_masks   = base_name + "-"      + md5_no_masks;
    const std::string key_with_masks = base_name +  "-LSM-" + md5_with_masks;

    std::map<std::string, WeightMatrix>::iterator j = matrix_cache.find(key_with_masks);
    if (j != matrix_cache.end()) {
        return (*j).second;
    }

    const std::string cache_key = (masks.active() && masks.cacheable()) ?
                                  key_with_masks
                                  : key_no_masks;

    // Shorten the key, to avoid "file name to long" errors


    // calculate weights matrix, apply mask if necessary

    eckit::Log::info() << "Elapsed 1 " << timer.elapsed()  << std::endl;

    here = timer.elapsed();
    WeightMatrix W(out.npts(), in.npts());
    eckit::Log::info() << "Create matrix " << timer.elapsed() - here << std::endl;

    bool caching = true;
    parametrisation_.get("caching", caching);

    // The WeightCache is parametrised by 'caching', as caching may be disabled on a field by field basis (unstructured grids)
    static caching::WeightCache cache;

    if (!caching || !cache.retrieve(cache_key, W)) {

        computeMatrixWeights(in, out, W);
        W.validate("computeMatrixWeights");

        if (masks.active() && masks.cacheable()) {
            applyMasks(W, masks);
            W.validate("applyMasks");
        }
        if (caching) {
            cache.insert(cache_key, W);
        }
    } else {
        W.validate("fromCache");
    }

    // If LSM not cacheabe, e.g. user provided, we apply the mask after
    if (masks.active() && !masks.cacheable())  {
        applyMasks(W, masks);
        W.validate("applyMasks");
    }

    here = timer.elapsed();
    std::swap(matrix_cache[key_with_masks], W);
    eckit::Log::info() << "Swap matrix " << timer.elapsed() - here << std::endl;

    return matrix_cache[key_with_masks];
}

lsm::LandSeaMasks MethodWeighted::getMasks(const atlas::Grid &in, const atlas::Grid &out) const {
    return lsm::LandSeaMasks::lookup(parametrisation_, in, out);
}

void MethodWeighted::execute(data::MIRField &field, const atlas::Grid &in, const atlas::Grid &out) const {

    eckit::Timer timer("MethodWeighted::execute");
    eckit::Log::info() << "MethodWeighted::execute" << std::endl;

    // setup sizes & checks
    const size_t npts_inp = in.npts();
    const size_t npts_out = out.npts();

    const WeightMatrix& W = getMatrix(in, out);

    ASSERT( W.rows() == npts_out );
    ASSERT( W.cols() == npts_inp );

    for (size_t i = 0; i < field.dimensions(); i++) {

        eckit::StrStream os;
        os << "Interpolating field ("  << eckit::BigNum(npts_inp) << " -> " << eckit::BigNum(npts_out) << ")" << eckit::StrStream::ends;
        std::string msg(os);
        eckit::Timer t(msg);

        // compute some statistics on the result
        // This is expensive so we might want to skip it in production code
        data::MIRFieldStats istats = field.statistics(i);

        const std::vector<double> &values = field.values(i);
        ASSERT(values.size() == npts_inp);


        // This should be local to the loop as field.value() will take ownership of result with std::swap()
        // For optimisation, one can also create result outside the loop, and resize() it here
        std::vector<double> result(npts_out);


        if ( field.hasMissing() ) {
            // Assumes compiler does return value optimization
            // otherwise we need to pass result matrix as parameter
            WeightMatrix MW = applyMissingValues(W, field, i);
            MW.validate("applyMissingValues");

            MW.multiply(values, result);
        } else {
            W.multiply(values, result);
        }

        field.values(result, i);  // Update field with result

        // compute some statistics on the result
        // This is expensive so we might want to skip it in production code
        eckit::Log::info() << "Input  Field statistics : " << istats << std::endl;

        data::MIRFieldStats ostats = field.statistics(i);
        eckit::Log::info() << "Output Field statistics : " << ostats << std::endl;

        /// FIXME: This assertion is to early in the case of LocalGrid input
        ///        because there will be output points which won't be updated (where skipped)
        ///        but later should be cropped out
        ///        UNLESS, we compute the statistics based on only points contained in the Domain

        if( in.domain().global() )
        {
            ASSERT(eckit::FloatCompare<double>::isApproximatelyGreaterOrEqual(ostats.minimum(), istats.minimum()));
            ASSERT(eckit::FloatCompare<double>::isApproximatelyGreaterOrEqual(istats.maximum(), ostats.maximum()));
        }

    }

    // TODO: move logic to MIRField
    // update if missing values are present
    if (field.hasMissing()) {
        const util::compare::is_equal_fn< double > check_miss(field.missingValue());
        bool still_has_missing = false;
        for (size_t i = 0; i < field.dimensions() && !still_has_missing; ++i) {
            const std::vector< double > &values = field.values(i);
            still_has_missing = (std::find_if(values.begin(), values.end(), check_miss) != values.end());
        }
        field.hasMissing(still_has_missing);
    }
}


void MethodWeighted::computeMatrixWeights(const atlas::Grid &in, const atlas::Grid &out, WeightMatrix &W) const {
    if (in.same(out)) {
        eckit::Log::info() << "Matrix is indentity" << std::endl;
        W.setIdentity();        // grids are the same, use identity matrix
    } else {
        eckit::Timer timer("Assemble matrix");
        assemble(W, in, out);   // assemble matrix of coefficients
        W.cleanup();
    }
}

WeightMatrix MethodWeighted::applyMissingValues(const WeightMatrix &W, data::MIRField &field, size_t which) const {

    ASSERT(field.hasMissing());


    // correct matrix weigths for the missing values (matrix copy happens here)
    const util::compare::is_equal_fn< double > check_miss(field.missingValue());
    const std::vector< double > &values = field.values(which);

    WeightMatrix X(W);

    for (size_t i = 0; i < X.rows(); i++) {

        // count missing values and accumulate weights
        double sum = 0.; // accumulated row weight, disregarding field missing values
        size_t Nmiss = 0;
        size_t Ncol  = 0;
        for (WeightMatrix::inner_const_iterator j(X, i); j; ++j, ++Ncol) {
            if (check_miss(values[j.col()]))
                ++Nmiss;
            else
                sum += *j;
        }
        const bool miss_some = (Nmiss > 0);
        const bool miss_all  = (Ncol == Nmiss);

        // redistribution
        if ( (miss_all || is_approx_zero(sum)) && (Ncol > 0)) {

            // all values are missing (or weights wrongly computed):
            // erase row & force missing value equality
            bool found = false;
            for (WeightMatrix::inner_iterator j(X, i); j; ++j) {
                *j = 0.;
                if (!found && check_miss(values[j.col()])) {
                    *j = 1.;
                    found = true;
                }
            }
            ASSERT(found);

        } else if ( miss_some ) {
            ASSERT(!is_approx_zero(sum));

            // apply linear redistribution
            for (WeightMatrix::inner_iterator j(X, i); j; ++j) {
                if (check_miss(values[j.col()])) {
                    *j = 0.;
                } else {
                    *j /= sum;
                }
            }

        }

    }

    return X;
}


void MethodWeighted::applyMasks(WeightMatrix &W, const lsm::LandSeaMasks &masks) const {

    eckit::Timer timer("MethodWeighted::applyMasks");

    eckit::Log::info() << "======== MethodWeighted::applyMasks(" << masks << ")" << std::endl;
    ASSERT(masks.active());

    const std::vector< bool > &imask = masks.inputMask();
    const std::vector< bool > &omask = masks.outputMask();

    eckit::Log::info() << "imask size " << imask.size() << std::endl;
    eckit::Log::info() << "omask size " << omask.size() << std::endl;

    eckit::Log::info() << "cols " << W.cols() << std::endl;
    eckit::Log::info() << "rows " << W.rows() << std::endl;

    ASSERT(imask.size() == W.cols());
    ASSERT(omask.size() == W.rows());


    // apply corrections on inequality != (XOR) of logical masks,
    // then redistribute weights
    // - output mask (omask) operates on matrix row index, here i
    // - input mask (imask) operates on matrix column index, here j.col()

    size_t fix = 0;
    for (size_t i = 0; i < W.rows(); i++) {

        ASSERT(i < omask.size());

        // correct weight of non-matching input point weight contribution
        double sum = 0.;
        bool row_changed = false;
        for (WeightMatrix::inner_iterator j(W, i); j; ++j) {

            ASSERT(j.col() < imask.size());

            if (omask[i] != imask[j.col()]) {
                *j *= lsmWeightAdjustement_;
                row_changed = true;
            }
            sum += *j;
        }

        // apply linear redistribution if necessary
        if (row_changed && !is_approx_zero(sum)) {
            ++fix;
            for (WeightMatrix::inner_iterator j(W, i); j; ++j)
                *j /= sum;
        }

    }


    // log corrections
    eckit::Log::info() << "MethodWeighted: applyMasks corrected " << eckit::BigNum(fix) << " out of " << eckit::Plural(W.rows() , "row") << std::endl;
}


void MethodWeighted::hash(eckit::MD5 &md5) const {
    md5.add(name());
}


}  // namespace method
}  // namespace mir

