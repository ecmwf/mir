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
/// @author Peter Bispham
/// @author Tiago Quintino
/// @author Pedro Maciel
///
/// @date May 2015


#include "mir/method/MethodWeighted.h"

#include <limits>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "eckit/log/Plural.h"
#include "eckit/log/ResourceUsage.h"
#include "eckit/types/FloatCompare.h"
#include "eckit/utils/MD5.h"

#include "mir/action/context/Context.h"
#include "mir/caching/InMemoryCache.h"
#include "mir/config/LibMir.h"
#include "mir/data/MIRField.h"
#include "mir/data/MIRFieldStats.h"
#include "mir/data/Space.h"
#include "mir/lsm/LandSeaMasks.h"
#include "mir/method/MatrixCacheCreator.h"
#include "mir/method/nonlinear/NonLinear.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Representation.h"
#include "mir/util/MIRStatistics.h"


namespace mir {
namespace method {


namespace {
static eckit::Mutex local_mutex;
static caching::InMemoryCache<WeightMatrix> matrix_cache("mirMatrix",
        512 * 1024 * 1024, 0,
        "$MIR_MATRIX_CACHE_MEMORY_FOOTPRINT");
}  // (anonymous namespace)


MethodWeighted::MethodWeighted(const param::MIRParametrisation& parametrisation) :
    Method(parametrisation) {
    ASSERT(parametrisation_.get("lsm-weight-adjustment", lsmWeightAdjustment_));

    pruneEpsilon_ = 0;
    ASSERT(parametrisation_.get("prune-epsilon", pruneEpsilon_));

    matrixValidate_ = eckit::Resource<bool>("$MIR_MATRIX_VALIDATE", false);
}


MethodWeighted::~MethodWeighted() = default;


void MethodWeighted::print(std::ostream &out) const {
    out <<  "cropping=" << cropping_
        << ",lsmWeightAdjustment=" << lsmWeightAdjustment_
        << ",pruneEpsilon=" << pruneEpsilon_;
}



bool MethodWeighted::sameAs(const Method& other) const {
    auto o = dynamic_cast<const MethodWeighted*>(&other);
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
                                  const Cropping& /*cropping*/) const {

    eckit::ResourceUsage usage(std::string("MethodWeighted::createMatrix [") + name() + "]");

    computeMatrixWeights(ctx, in, out, W);

    // matrix validation always happens after creation,
    // because the matrix can/will be cached
    W.validate("computeMatrixWeights");

    if (masks.active() && masks.cacheable()) {
        applyMasks(W, masks);
        if (matrixValidate_) {
            W.validate("applyMasks");
        }
    }
}

// This returns a 'const' matrix so we ensure that we don't change it and break the in-memory cache
const WeightMatrix& MethodWeighted::getMatrix(context::Context& ctx,
        const repres::Representation& in,
        const repres::Representation& out) const {

    eckit::AutoLock<eckit::Mutex> lock(local_mutex);
    auto& log = eckit::Log::debug<LibMir>();

    log << "MethodWeighted::getMatrix " << *this << std::endl;
    eckit::TraceTimer<LibMir> timer("MethodWeighted::getMatrix");


    double here = timer.elapsed();
    const lsm::LandSeaMasks masks = getMasks(in, out);
    log << "MethodWeighted::getMatrix land-sea masks: " << timer.elapsed() - here << "s, " << (masks.active() ? "active" : "not active") << std::endl;


    here = timer.elapsed();
    const std::string& shortName_in  = in.uniqueName();
    const std::string& shortName_out = out.uniqueName();

    // TODO: add (possibly) missing unique identifiers
    // NOTE: key has to be relatively short, to avoid filesystem "File name too long" errors
    // Check with $getconf -a | grep -i name
    std::string disk_key = std::string(name()) + "/" + shortName_in + "/" + shortName_out + "-";
    eckit::MD5 hash;
    hash << *this
         << shortName_in
         << shortName_out
         << in.boundingBox()
         << out.boundingBox()
         << pruneEpsilon_
         << lsmWeightAdjustment_;

    disk_key += hash;
    std::string memory_key = disk_key;

    // Add masks if any
    if (masks.active()) {

        std::string masks_key = "-lsm-";
        masks_key += masks.cacheName();

        memory_key += masks_key;

        if (masks.cacheable()) {
            disk_key += masks_key;
        }
    }


    {
        auto j = matrix_cache.find(memory_key);
        const bool found = j != matrix_cache.end();
        log << "MethodWeighted::getMatrix cache key: " << memory_key << " " << timer.elapsed() - here << "s, " << (found ? "found" : "not found") << " in memory cache" << std::endl;
        if (found) {
            const WeightMatrix& mat = *j;
            log << "Using matrix from InMemoryCache " <<  mat << std::endl;
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
        cache.getOrCreate(disk_key, creator, W);

    } else {
        createMatrix(ctx, in, out, W, masks, cropping_);
    }

    // If LSM not cacheable to disk, because it is user provided
    // it will be cached in memory nevertheless
    if (masks.active() && !masks.cacheable())  {
        applyMasks(W, masks);
        if (matrixValidate_) {
            W.validate("applyMasks");
        }
    }

    log << "MethodWeighted::getMatrix create weights matrix: " << timer.elapsed() - here << "s" << std::endl;
    log << "MethodWeighted::getMatrix matrix W " << W << std::endl;

    // insert matrix in the in-memory cache and update memory footprint

    WeightMatrix& w = matrix_cache[memory_key];
    W.swap(w);

    size_t footprint = w.footprint();
    caching::InMemoryCacheUsage usage(w.inSharedMemory() ? 0 : footprint, w.inSharedMemory() ? footprint : 0);

    log << "Matrix footprint " << w.owner() << " " << usage << " W -> " << W.owner() << std::endl;

    matrix_cache.footprint(memory_key, usage);
    return w;
}


void MethodWeighted::setOperandMatricesFromVectors(
    WeightMatrix::Matrix& A,
    WeightMatrix::Matrix& B,
    const MIRValuesVector& Avector,
    const MIRValuesVector& Bvector,
    const double& missingValue,
    const data::Space& space ) const {

    // set input matrix B (from A = W × B)
    // FIXME: remove const_cast once Matrix provides read-only view
    WeightMatrix::Matrix Bwrap(const_cast<double *>(Bvector.data()), Bvector.size(), 1);

    space.linearise(Bwrap, B, missingValue);

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
    MIRValuesVector& Avector,
    const double& missingValue,
    const data::Space& space ) const {

    // set output vector A (from A = W × B)
    // FIXME: remove const_cast once Matrix provides read-only view
    ASSERT(Avector.size() == A.rows());
    WeightMatrix::Matrix Awrap(const_cast<double *>(Avector.data()), Avector.size(), 1);

    space.unlinearise(A, Awrap, missingValue);
}


lsm::LandSeaMasks MethodWeighted::getMasks(const repres::Representation& in, const repres::Representation & out) const {
    return lsm::LandSeaMasks::lookup(parametrisation_, in, out);
}


void MethodWeighted::execute(context::Context& ctx, const repres::Representation& in, const repres::Representation& out) const {


    // Make sure another thread to no evict anything from the cache while we are using it
    caching::InMemoryCacheUser<WeightMatrix> matrix_use(matrix_cache, ctx.statistics().matrixCache_);


    static bool check_stats = eckit::Resource<bool>("mirCheckStats", false);

    eckit::TraceTimer<LibMir> timer("MethodWeighted::execute");
    auto& log = eckit::Log::debug<LibMir>();
    log << "MethodWeighted::execute" << std::endl;

    // setup sizes & checks
    const size_t npts_inp = in.numberOfPoints();
    const size_t npts_out = out.numberOfPoints();

    const WeightMatrix& W = getMatrix(ctx, in, out);
    ASSERT( W.rows() == npts_out );
    ASSERT( W.cols() == npts_inp );

    data::MIRField& field = ctx.field();
    const bool hasMissing = field.hasMissing();
    const double missingValue = hasMissing ? field.missingValue() : std::numeric_limits<double>::quiet_NaN();

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

        // Get input/output matrices
        std::string space;
        parametrisation_.get("space", space);
        const data::Space& sp = data::SpaceChooser::lookup(space);

        MIRValuesVector result(npts_out);  // field.update() takes ownership with std::swap()
        WeightMatrix::Matrix mi;
        WeightMatrix::Matrix mo;
        setOperandMatricesFromVectors(mo, mi, result, field.values(i), missingValue, sp);
        ASSERT(mi.rows() == npts_inp);
        ASSERT(mo.rows() == npts_out);

        {
            eckit::AutoTiming timing(ctx.statistics().timer_, ctx.statistics().matrixTiming_);
            eckit::Timer t("Matrix-Multiply-hasMissing-" + std::to_string(hasMissing), log);

            if (hasMissing) {

                WeightMatrix M;
                std::string nl = "missing-if-heaviest-missing";

                std::unique_ptr<const nonlinear::NonLinear> nonlin(nonlinear::NonLinearFactory::build(nl, parametrisation_));
                {
                    eckit::Timer t("non-linear treatment: " + nl, eckit::Log::debug<LibMir>());
                    WeightMatrix X(W);

                    if (nonlin->treatment(mi, X, mo, field.values(i), missingValue)) {
                        if (matrixValidate_) {
                            M.validate(nl.c_str());
                        }
                        X.swap(M);
                    }
                }


                M.multiply(mi, mo);
            } else {
                W.multiply(mi, mo);
            }
        }

        // update field values with interpolation result
        setVectorFromOperandMatrix(mo, result, missingValue, sp);
        field.update(result, i, hasMissing || canIntroduceMissingValues());


        if (check_stats) {
            // compute some statistics on the result
            data::MIRFieldStats ostats = field.statistics(i);
            log << "Input field statistics:  " << istats << std::endl;
            log << "Output field statistics: " << ostats << std::endl;

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


void MethodWeighted::applyMasks(WeightMatrix& W, const lsm::LandSeaMasks& masks) const {

    eckit::TraceTimer<LibMir> timer("MethodWeighted::applyMasks");
    auto& log = eckit::Log::debug<LibMir>();

    log << "======== MethodWeighted::applyMasks(" << masks << ")" << std::endl;

    ASSERT(masks.active());

    const std::vector< bool >& imask = masks.inputMask();
    const std::vector< bool >& omask = masks.outputMask();

    log << "imask size=" << imask.size() << " == #cols=" << W.cols() << std::endl;
    log << "omask size " << omask.size() << " == #rows=" << W.rows() << std::endl;

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
    log << "MethodWeighted: applyMasks corrected "
        << eckit::BigNum(fix)
        << " out of "
        << eckit::Plural(W.rows() , "output point")
        << std::endl;
}


void MethodWeighted::hash(eckit::MD5& md5) const {
    md5.add(name());
}


bool MethodWeighted::canCrop() const {
    return true;
}


void MethodWeighted::setCropping(const util::BoundingBox& bbox) {
    cropping_.boundingBox(bbox);
}


bool MethodWeighted::hasCropping() const {
    return cropping_;
}


const util::BoundingBox& MethodWeighted::getCropping() const {
    return cropping_.boundingBox();
}


bool MethodWeighted::canIntroduceMissingValues() const {
    return false;
}


}  // namespace method
}  // namespace mir

