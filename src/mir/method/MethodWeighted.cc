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
#include "eckit/utils/MD5.h"
#include "atlas/grid.h"
#include "atlas/mesh/Mesh.h"
#include "mir/action/context/Context.h"
#include "mir/caching/InMemoryCache.h"
#include "mir/config/LibMir.h"
#include "mir/data/MIRField.h"
#include "mir/data/MIRFieldStats.h"
#include "mir/lsm/LandSeaMasks.h"
#include "mir/method/decompose/Decompose.h"
#include "mir/repres/Representation.h"
#include "mir/util/Compare.h"
#include "mir/util/MIRGrid.h"
#include "mir/util/MIRStatistics.h"

using mir::util::compare::is_approx_zero;
using mir::util::compare::is_approx_one;


namespace mir {
namespace method {


namespace {
static eckit::Mutex local_mutex;
static InMemoryCache<WeightMatrix> matrix_cache("mirMatrix",
        512 * 1024 * 1024,
        "$MIR_MATRIX_CACHE_MEMORY_FOOTPRINT");
}  // (anonymous namespace)


MethodWeighted::MethodWeighted(const param::MIRParametrisation& parametrisation) :
    Method(parametrisation) {
    ASSERT(parametrisation.get("lsm-weight-adjustment", lsmWeightAdjustement_));

    pruneEpsilon_ = 0;
    ASSERT(parametrisation_.get("prune-epsilon", pruneEpsilon_));
}


MethodWeighted::~MethodWeighted() {
}


void MethodWeighted::createMatrix(context::Context& ctx,
                                  const repres::Representation& in,
                                  const repres::Representation& out,
                                  WeightMatrix& W,
                                  const lsm::LandSeaMasks& masks) const {

    computeMatrixWeights(ctx, in, out, W);

    W.validate("computeMatrixWeights");

    if (masks.active() && masks.cacheable()) {
        applyMasks(W, masks);
        W.validate("applyMasks");
    }
}

// This returns a 'const' matrix so we ensure that we don't change it and break the in-memory cache
const WeightMatrix& MethodWeighted::getMatrix(context::Context& ctx,
        const repres::Representation& rin,
        const repres::Representation& rout) const {

    atlas::Grid gin(rin.grid());
    atlas::Grid gout(rout.grid());

    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    eckit::Log::debug<LibMir>() << "MethodWeighted::getMatrix " << *this << std::endl;
    eckit::TraceTimer<LibMir> timer("MethodWeighted::getMatrix");


    double here = timer.elapsed();
    const lsm::LandSeaMasks masks = getMasks(rin, rout);
    eckit::Log::debug<LibMir>() << "Compute LandSeaMasks " << timer.elapsed() - here << std::endl;

    eckit::Log::debug<LibMir>() << "++++ LSM masks " << masks << std::endl;
    here = timer.elapsed();

    // TODO: add (possibly) missing unique identifiers
    // NOTE: key has to be relatively short, to avoid filesystem "File name too long" errors
    // Check with $getconf -a | grep -i name
    eckit::MD5 md5;
    md5 << *this
        << gin
        << gout
        << pruneEpsilon_
        << lsmWeightAdjustement_;

    const eckit::MD5::digest_t md5_no_masks(md5.digest());
    md5 << masks;
    const eckit::MD5::digest_t md5_with_masks(md5.digest());
    eckit::Log::debug<LibMir>() << "Compute md5 " << timer.elapsed() - here << std::endl;


    const std::string shortName_in  = rin.uniqueName();
    const std::string shortName_out = rout.uniqueName();
    ASSERT(!shortName_in.empty());
    ASSERT(!shortName_out.empty());

    const std::string base_name = std::string(name()) + "-" + shortName_in + "-" + shortName_out;
    const std::string key_no_masks   = base_name + "-"      + md5_no_masks;
    const std::string key_with_masks = base_name +  "-LSM-" + md5_with_masks;


    InMemoryCache<WeightMatrix>::iterator j = matrix_cache.find(key_with_masks);
    if (j != matrix_cache.end()) {
        return *j;
    }

    const std::string cache_key = (masks.active() && masks.cacheable()) ? key_with_masks : key_no_masks;



    // calculate weights matrix, apply mask if necessary

    eckit::Log::debug<LibMir>() << "Elapsed 1 " << timer.elapsed()  << std::endl;

    here = timer.elapsed();
    WeightMatrix W(gout.size(), gin.size());
    eckit::Log::debug<LibMir>() << "Create matrix " << timer.elapsed() - here << std::endl;

    bool caching = true;
    parametrisation_.get("caching", caching);

    eckit::PathName path;

    if (caching) {

        /// The WeightCache is parametrised by 'caching',
        /// as caching may be disabled on a field by field basis (unstructured grids)
        static caching::WeightCache cache(parametrisation_);

        class MatrixCacheCreator: public caching::WeightCache::CacheContentCreator {

            const MethodWeighted& owner_;
            context::Context& ctx_;
            const repres::Representation& in_;
            const repres::Representation& out_;
            const lsm::LandSeaMasks& masks_;

            virtual void create(const eckit::PathName& path, WeightMatrix& W) {
                owner_.createMatrix(ctx_, in_, out_, W, masks_);
            }

        public:
            MatrixCacheCreator(const MethodWeighted& owner,
                               context::Context& ctx,
                               const repres::Representation& in,
                               const repres::Representation& out,
                               const lsm::LandSeaMasks& masks):
                owner_(owner),
                ctx_(ctx),
                in_(in),
                out_(out),
                masks_(masks) {}
        };

        MatrixCacheCreator creator(*this, ctx, rin, rout, masks);
        path = cache.getOrCreate(cache_key, creator, W);

    }
    else {
        createMatrix(ctx, rin, rout, W, masks);
    }


    // If LSM not cacheabe, e.g. user provided, we apply the mask after
    if (masks.active() && !masks.cacheable())  {
        applyMasks(W, masks);
        W.validate("applyMasks");
    }

    // inserts the matrix in the cache
    WeightMatrix& w = matrix_cache[key_with_masks];
    std::swap(w, W);

    // update memory footprint
    matrix_cache.footprint(key_with_masks, w.footprint());

    return w;
}


void MethodWeighted::setOperandMatricesFromVectors(
    WeightMatrix::Matrix& A,
    WeightMatrix::Matrix& B,
    const std::vector<double>& Avector,
    const std::vector<double>& Bvector,
    const double& missingValue ) const {

    // set input matrix B (from A = W × B)
    // FIXME: remove const_cast once Matrix provides read-only view
    WeightMatrix::Matrix Bwrap(const_cast<double *>(Bvector.data()), Bvector.size(), 1);

    std::string decomposition;
    parametrisation_.get("decomposition", decomposition);

    const decompose::Decompose& decomp = decompose::DecomposeChooser::lookup(decomposition);
    decomp.setMissingValue(missingValue);
    decomp.decompose(Bwrap, B);

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
    const double& missingValue) const {

    // set output vector A (from A = W × B)
    // FIXME: remove const_cast once Matrix provides read-only view
    ASSERT(Avector.size() == A.rows());
    WeightMatrix::Matrix Awrap(const_cast<double *>(Avector.data()), Avector.size(), 1);

    std::string decomposition;
    parametrisation_.get("decomposition", decomposition);

    const decompose::Decompose& decomp = decompose::DecomposeChooser::lookup(decomposition);
    decomp.setMissingValue(missingValue);
    decomp.recompose(A, Awrap);
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
        WeightMatrix::Matrix mi;
        WeightMatrix::Matrix mo;
        setOperandMatricesFromVectors(mo, mi, result, field.values(i), missingValue);
        ASSERT(mi.rows() == npts_inp);
        ASSERT(mo.rows() == npts_out);

        if (field.hasMissing()) {

            eckit::AutoTiming timing(ctx.statistics().timer_, ctx.statistics().matrixTiming_);

            eckit::Timer t("Matrix-Multiply-MissingValues", eckit::Log::debug<LibMir>());

            WeightMatrix MW;
            applyMissingValues(W, field.values(i), field.missingValue(), MW); // Don't assume compiler can do return value optimization !!!

            MW.multiply(mi, mo);

        } else {

            eckit::AutoTiming timing(ctx.statistics().timer_, ctx.statistics().matrixTiming_);

            eckit::Timer t("Matrix-Multiply-Standard", eckit::Log::debug<LibMir>());

            W.multiply(mi, mo);

        }

        // update field values with interpolation result
        setVectorFromOperandMatrix(mo, result, missingValue);
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

            if (in.domain().isGlobal()) {
                ASSERT(eckit::types::is_approximately_greater_or_equal(ostats.minimum(), istats.minimum()));
                ASSERT(eckit::types::is_approximately_greater_or_equal(istats.maximum(), ostats.maximum()));
            }
        }

    }

    // TODO: move logic to MIRField
    // update if missing values are present
    if (field.hasMissing()) {
        const util::compare::IsMissingFn isMissing(field.missingValue());
        bool still_has_missing = false;
        for (size_t i = 0; i < field.dimensions() && !still_has_missing; ++i) {
            const std::vector< double >& values = field.values(i);
            still_has_missing = (std::find_if(values.begin(), values.end(), isMissing) != values.end());
        }
        field.hasMissing(still_has_missing);
    }
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

                const double factor = is_approx_zero(sum) ? 0 : 1. / sum;
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
                *it *= lsmWeightAdjustement_;
                row_changed = true;
            }
            sum += *it;
        }

        // apply linear redistribution if necessary
        if (row_changed && !is_approx_zero(sum)) {
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
                                << eckit::Plural(W.rows() , "row")
                                << std::endl;
}


void MethodWeighted::hash(eckit::MD5& md5) const {
    md5.add(name());
}


}  // namespace method
}  // namespace mir

