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
/// @author Peter Bispham
/// @author Tiago Quintino
/// @author Pedro Maciel
///
/// @date May 2015


#include "mir/method/MethodWeighted.h"

#include <algorithm>
#include <map>
#include <sstream>
#include <string>

#include "eckit/log/Plural.h"
#include "eckit/log/ResourceUsage.h"
#include "eckit/types/FloatCompare.h"
#include "eckit/utils/MD5.h"

#include "mir/action/context/Context.h"
#include "mir/caching/InMemoryCache.h"
#include "mir/config/LibMir.h"
#include "mir/data/Dimension.h"
#include "mir/data/MIRField.h"
#include "mir/data/MIRFieldStats.h"
#include "mir/lsm/LandSeaMasks.h"
#include "mir/repres/Representation.h"
#include "mir/util/MIRStatistics.h"

#include "mir/method/MatrixCacheCreator.h"
#include "mir/method/Cropping.h"

namespace mir {
namespace method {


namespace {
static eckit::Mutex local_mutex;
static InMemoryCache<WeightMatrix> matrix_cache("mirMatrix",
        512 * 1024 * 1024, 0,
        "$MIR_MATRIX_CACHE_MEMORY_FOOTPRINT");
}  // (anonymous namespace)


MethodWeighted::MethodWeighted(const param::MIRParametrisation& parametrisation) :
    Method(parametrisation) {
    ASSERT(parametrisation_.get("lsm-weight-adjustment", lsmWeightAdjustment_));

    pruneEpsilon_ = 0;
    ASSERT(parametrisation_.get("prune-epsilon", pruneEpsilon_));
}


MethodWeighted::~MethodWeighted() {
}

void MethodWeighted::print(std::ostream &out) const {
    out << "cropping="
        << cropping_
        << ",lsmWeightAdjustment="
        << lsmWeightAdjustment_
        << ",pruneEpsilon="
        << pruneEpsilon_;
}



bool MethodWeighted::sameAs(const Method& other) const {
    const MethodWeighted* o = dynamic_cast<const MethodWeighted*>(&other);
    return o
           && (lsmWeightAdjustment_ == o->lsmWeightAdjustment_)
           && (pruneEpsilon_ == o->pruneEpsilon_)
           && lsm::LandSeaMasks::sameLandSeaMasks(parametrisation_, o->parametrisation_)
           && cropping_ == o->cropping_;

}


// Called from MatrixCacheCreator when a matrix in not found in disk cache
void MethodWeighted::createMatrix(context::Context& ctx,
                                  const repres::Representation& in,
                                  const repres::Representation& out,
                                  WeightMatrix& W,
                                  const lsm::LandSeaMasks& masks,
                                  const Cropping& cropping) const {

    eckit::ResourceUsage usage(std::string("MethodWeighted::createMatrix [") + name() + "]");

    computeMatrixWeights(ctx, in, out, W);

    W.validate("computeMatrixWeights");

    if (masks.active() && masks.cacheable()) {
        applyMasks(W, masks);
        W.validate("applyMasks");
    }
}

// This returns a 'const' matrix so we ensure that we don't change it and break the in-memory cache
const WeightMatrix& MethodWeighted::getMatrix(context::Context& ctx,
        const repres::Representation& in,
        const repres::Representation& out) const {

    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    eckit::Log::debug<LibMir>() << "MethodWeighted::getMatrix " << *this << std::endl;
    eckit::TraceTimer<LibMir> timer("MethodWeighted::getMatrix");


    double here = timer.elapsed();
    const lsm::LandSeaMasks masks = getMasks(in, out);
    eckit::Log::debug<LibMir>() << "MethodWeighted::getMatrix land-sea masks: " << timer.elapsed() - here << "s, " << (masks.active() ? "active" : "not active") << std::endl;


    here = timer.elapsed();
    const std::string shortName_in  = in.uniqueName();
    const std::string shortName_out = out.uniqueName();

    // TODO: add (possibly) missing unique identifiers
    // NOTE: key has to be relatively short, to avoid filesystem "File name too long" errors
    // Check with $getconf -a | grep -i name
    std::string key = std::string(name()) + "/" + shortName_in + "/" + shortName_out;
    eckit::MD5 hash;
    hash << *this
         << shortName_in
         << shortName_out
         << pruneEpsilon_
         << lsmWeightAdjustment_;

    if (masks.active() && masks.cacheable()) {
        hash << masks;
        key += "-lsm/";
    }


    key += std::string(hash);

    {
        InMemoryCache<WeightMatrix>::iterator j = matrix_cache.find(key);
        const bool found = j != matrix_cache.end();
        eckit::Log::debug<LibMir>() << "MethodWeighted::getMatrix cache key: " << key << " " << timer.elapsed() - here << "s, " << (found ? "found" : "not found") << " in memory cache" << std::endl;
        if (found) {
            const WeightMatrix& mat = *j;
            eckit::Log::debug<LibMir>() << "Using matrix from InMemoryCache " <<  mat << std::endl;
            return mat;
        }
    }


    // calculate weights matrix, apply mask if necessary
    here = timer.elapsed();
    WeightMatrix W(out.numberOfPoints(), in.numberOfPoints());

    bool caching = true;
    parametrisation_.get("caching", caching);
    if (caching) {

        // The WeightCache is parametrised by 'caching',
        // as caching may be disabled on a field by field basis (unstructured grids)
        static caching::WeightCache cache(parametrisation_);
        MatrixCacheCreator creator(*this, ctx, in, out, masks, cropping_);
        cache.getOrCreate(key, creator, W);

    } else {
        createMatrix(ctx, in, out, W, masks, cropping_);
    }

    // If LSM not cacheable, e.g. user provided, we apply the mask after
    if (masks.active() && !masks.cacheable())  {
        applyMasks(W, masks);
        W.validate("applyMasks");
    }

    eckit::Log::debug<LibMir>() << "MethodWeighted::getMatrix create weights matrix: " << timer.elapsed() - here << "s" << std::endl;
    eckit::Log::debug<LibMir>() << "MethodWeighted::getMatrix matrix W " << W << std::endl;

    // insert matrix in the in-memory cache and update memory footprint

    WeightMatrix& w = matrix_cache[key];
    W.swap(w);

    size_t footprint = w.footprint();
    InMemoryCacheUsage usage(w.inSharedMemory() ? 0 : footprint, w.inSharedMemory() ? footprint : 0);

    eckit::Log::info() << "Matrix footprint " << w.owner() << " " << usage << " W -> " << W.owner() << std::endl;

    matrix_cache.footprint(key, usage);
    return w;
}


void MethodWeighted::setOperandMatricesFromVectors(
    WeightMatrix::Matrix& A,
    WeightMatrix::Matrix& B,
    const std::vector<double>& Avector,
    const std::vector<double>& Bvector,
    const double& missingValue,
    const data::Dimension& dimension ) const {

    // set input matrix B (from A = W × B)
    // FIXME: remove const_cast once Matrix provides read-only view
    WeightMatrix::Matrix Bwrap(const_cast<double *>(Bvector.data()), Bvector.size(), 1);

    dimension.linearise(Bwrap, B, missingValue);

    // set output matrix A (from A = W × B)
    // reuses output values vector if handling a column vector, otherwise allocates new matrix
    if (B.cols() == 1) {

        // FIXME: remove const_cast once Matrix provides read-only view
        WeightMatrix::Matrix Awrap(const_cast<double *>(Avector.data()), Avector.size(), 1);
        A.swap(Awrap);

    } else {

        WeightMatrix::Matrix Awrap(Avector.size(), B.cols());
        Awrap.setZero();
        A.swap(Awrap);

    }
}


void MethodWeighted::setVectorFromOperandMatrix(
    const WeightMatrix::Matrix& A,
    std::vector<double>& Avector,
    const double& missingValue,
    const data::Dimension& dimension ) const {

    // set output vector A (from A = W × B)
    // FIXME: remove const_cast once Matrix provides read-only view
    ASSERT(Avector.size() == A.rows());
    WeightMatrix::Matrix Awrap(const_cast<double *>(Avector.data()), Avector.size(), 1);

    dimension.unlinearise(A, Awrap, missingValue);
}


lsm::LandSeaMasks MethodWeighted::getMasks(const repres::Representation& in, const repres::Representation & out) const {
    return lsm::LandSeaMasks::lookup(parametrisation_, in, out);
}


void MethodWeighted::execute(context::Context& ctx, const repres::Representation& in, const repres::Representation& out) const {


    // Make sure another thread to no evict anything from the cache while we are using it
    InMemoryCacheUser<WeightMatrix> matrix_use(matrix_cache, ctx.statistics().matrixCache_);


    static bool check_stats = eckit::Resource<bool>("mirCheckStats", false);

    eckit::TraceTimer<LibMir> timer("MethodWeighted::execute");
    eckit::Log::debug<LibMir>() << "MethodWeighted::execute" << std::endl;

    // setup sizes & checks
    const size_t npts_inp = in.numberOfPoints();
    const size_t npts_out = out.numberOfPoints();

    const WeightMatrix& W = getMatrix(ctx, in, out);
    ASSERT( W.rows() == npts_out );
    ASSERT( W.cols() == npts_inp );

    data::MIRField& field = ctx.field();
    const double missingValue = field.hasMissing() ? field.missingValue() : std::numeric_limits<double>::quiet_NaN();

    for (size_t i = 0; i < field.dimensions(); i++) {

        std::ostringstream os;
        os << "Interpolating field ("  << eckit::BigNum(npts_inp) << " -> " << eckit::BigNum(npts_out) << ")";
        eckit::TraceTimer<LibMir> t(os.str());

        // compute some statistics on the result
        // This is expensive so we might want to skip it in production code
        data::MIRFieldStats istats;

        if (check_stats) {
            istats = field.statistics(i);
        }


        // results should be local to the loop as field.update() will take ownership of result with std::swap()
        // For optimisation, one can also create result outside the loop, and resize() it here
        std::vector<double> result(npts_out);

        // Get input/output matrices
        std::string dimension;
        parametrisation_.get("dimension", dimension);
        const data::Dimension& dim = data::DimensionChooser::lookup(dimension);

        WeightMatrix::Matrix mi;
        WeightMatrix::Matrix mo;
        setOperandMatricesFromVectors(mo, mi, result, field.values(i), missingValue, dim);
        ASSERT(mi.rows() == npts_inp);
        ASSERT(mo.rows() == npts_out);

        {
            eckit::AutoTiming timing(ctx.statistics().timer_, ctx.statistics().matrixTiming_);
            eckit::Timer t("Matrix-Multiply-hasMissing-" + std::to_string(field.hasMissing()), eckit::Log::debug<LibMir>());

            if (field.hasMissing()) {
                WeightMatrix M;
                applyMissingValues(W, field.values(i), field.missingValue(), M); // Don't assume compiler can do return value optimization !!!

                M.multiply(mi, mo);
            } else {
                W.multiply(mi, mo);
            }
        }

        // update field values with interpolation result
        setVectorFromOperandMatrix(mo, result, missingValue, dim);
        field.update(result, i);


        if (check_stats) {
            // compute some statistics on the result
            // This is expensive so we might want to skip it in production code
            eckit::Log::debug<LibMir>() << "Input  Field statistics : " << istats << std::endl;

            data::MIRFieldStats ostats = field.statistics(i);
            eckit::Log::debug<LibMir>() << "Output Field statistics : " << ostats << std::endl;

            /// FIXME: This assertion is to early in the case of LocalGrid input
            ///        because there will be output points which won't be updated (where skipped)
            ///        but later should be cropped out
            ///        UNLESS, we compute the statistics based on only points contained in the Domain

            if (in.isGlobal()) {
                ASSERT(eckit::types::is_approximately_greater_or_equal(ostats.minimum(), istats.minimum()));
                ASSERT(eckit::types::is_approximately_greater_or_equal(istats.maximum(), ostats.maximum()));
            }
        }

    }

    // interpolation could change if missing values are (still) present, re-check
    field.checkMissing();
}


void MethodWeighted::computeMatrixWeights(context::Context& ctx,
        const repres::Representation& in,
        const repres::Representation& out,
        WeightMatrix& W) const {
    eckit::AutoTiming timing(ctx.statistics().timer_, ctx.statistics().computeMatrixTiming_);

    if (in.sameAs(out)) {
        eckit::Log::debug<LibMir>() << "Matrix is indentity" << std::endl;
        W.setIdentity(W.rows(), W.cols());
    } else {
        eckit::TraceTimer<LibMir> timer("Assemble matrix");
        assemble(ctx.statistics(), W, in, out);
        W.cleanup(pruneEpsilon_);
    }
}


void MethodWeighted::applyMissingValues(
    const WeightMatrix& W,
    const std::vector<double>& values,
    const double& missingValue,
    WeightMatrix& MW) const {

    eckit::Timer t1("applyMissingValues", eckit::Log::debug<LibMir>());

    // correct matrix weigths for the missing values (matrix copy happens here)
    ASSERT( W.cols() == values.size() );
    WeightMatrix X(W);

    WeightMatrix::iterator it(X);
    for (WeightMatrix::Size i = 0; i < X.rows(); i++) {
        const WeightMatrix::iterator begin = X.begin(i);
        const WeightMatrix::iterator end   = X.end(i);

        // count missing values, accumulate weights (disregarding missing values) and find closest value (maximum weight)
        size_t Nmissing = 0;
        size_t Nentries = 0;
        double sum = 0.;
        WeightMatrix::const_iterator closest = begin;

        for (it = begin; it != end; ++it, ++Nentries) {
            if (values[it.col()] == missingValue)
                ++Nmissing;
            else
                sum += *it;
            if (*closest < *it)
                closest = it;
        }

        // weights redistribution: zero-weight all missing values, linear re-weighting for the others;
        // if all values are missing, or the closest value is missing, force missing value
        if (Nmissing > 0) {
            if (Nmissing == Nentries || values[closest.col()] == missingValue) {

                bool found = false;
                for (it = begin; it != end; ++it) {
                    *it = 0.;
                    if (values[it.col()] == missingValue && !found) {
                        *it = 1.;
                        found = true;
                    }
                }
                ASSERT(found);

            } else {

                const double factor = eckit::types::is_approximately_equal(sum, 0.) ? 0 : 1. / sum;
                for (it = begin; it != end; ++it) {
                    if (values[it.col()] == missingValue) {
                        *it = 0.;
                    } else {
                        *it *= factor;
                    }
                }

            }
        }

    }

    X.validate("MethodWeighted::applyMissingValues");

    MW.swap(X);
}


void MethodWeighted::applyMasks(WeightMatrix& W, const lsm::LandSeaMasks& masks) const {

    eckit::TraceTimer<LibMir> timer("MethodWeighted::applyMasks");
    eckit::Log::debug<LibMir>() << "======== MethodWeighted::applyMasks(" << masks << ")" << std::endl;

    ASSERT(masks.active());

    const std::vector< bool >& imask = masks.inputMask();
    const std::vector< bool >& omask = masks.outputMask();

    eckit::Log::debug<LibMir>() << "imask size " << imask.size() << std::endl;
    eckit::Log::debug<LibMir>() << "omask size " << omask.size() << std::endl;

    eckit::Log::debug<LibMir>() << "cols " << W.cols() << std::endl;
    eckit::Log::debug<LibMir>() << "rows " << W.rows() << std::endl;

    ASSERT(imask.size() == W.cols());
    ASSERT(omask.size() == W.rows());


    // apply corrections on inequality != (XOR) of logical masks,
    // then redistribute weights
    // - output mask (omask) operates on matrix row index, here i
    // - input mask (imask) operates on matrix column index, here j.col()

    WeightMatrix::iterator it(W);
    size_t fix = 0;
    for (size_t i = 0; i < W.rows(); i++) {

        ASSERT(i < omask.size());

        // correct weight of non-matching input point weight contribution
        double sum = 0.;
        bool row_changed = false;
        for (it = W.begin(i); it != W.end(i); ++it) {

            ASSERT(it.col() < imask.size());

            if (omask[i] != imask[it.col()]) {
                *it *= lsmWeightAdjustment_;
                row_changed = true;
            }
            sum += *it;
        }

        // apply linear redistribution if necessary
        if (row_changed && !eckit::types::is_approximately_equal(sum, 0.)) {
            ++fix;
            for (it = W.begin(i); it != W.end(i); ++it) {
                *it /= sum;
            }
        }

    }

    // log corrections
    eckit::Log::debug<LibMir>() << "MethodWeighted: applyMasks corrected "
                                << eckit::BigNum(fix)
                                << " out of "
                                << eckit::Plural(W.rows() , "output point")
                                << std::endl;
}


void MethodWeighted::hash(eckit::MD5& md5) const {
    md5.add(name());
}

void MethodWeighted::setCropping(const util::BoundingBox& bbox) {
    cropping_.boundingBox(bbox);

}

bool MethodWeighted::canCrop() const {
    return true;
}


const repres::Representation* MethodWeighted::adjustOutputRepresentation(const repres::Representation* representation) {

    if(cropping_.active()) {
        repres::RepresentationHandle out(representation); // Will destroy represenation
        return representation->cropped(cropping_.boundingBox());
    }


    return representation;
}


}  // namespace method
}  // namespace mir

