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


#include "mir/method/MethodWeighted.h"

#include <limits>
#include <mutex>
#include <sstream>
#include <string>

#include "eckit/types/FloatCompare.h"
#include "eckit/utils/MD5.h"
#include "eckit/utils/StringTools.h"

#include "mir/action/context/Context.h"
#include "mir/caching/InMemoryCache.h"
#include "mir/config/LibMir.h"
#include "mir/data/MIRField.h"
#include "mir/data/MIRFieldStats.h"
#include "mir/data/Space.h"
#include "mir/lsm/LandSeaMasks.h"
#include "mir/method/MatrixCacheCreator.h"
#include "mir/method/nonlinear/NonLinear.h"
#include "mir/method/solver/Multiply.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Representation.h"
#include "mir/util/Log.h"
#include "mir/util/MIRStatistics.h"
#include "mir/util/Trace.h"
#include "mir/util/Types.h"


namespace mir {
namespace method {


static std::mutex local_mutex;

constexpr size_t CAPACITY = 512 * 1024 * 1024;
static caching::InMemoryCache<WeightMatrix> matrix_cache("mirMatrix", CAPACITY, 0,
                                                         "$MIR_MATRIX_CACHE_MEMORY_FOOTPRINT");


MethodWeighted::MethodWeighted(const param::MIRParametrisation& parametrisation) :
    Method(parametrisation), solver_(new solver::Multiply(parametrisation)) {
    ASSERT(parametrisation_.get("lsm-weight-adjustment", lsmWeightAdjustment_));

    pruneEpsilon_ = 0;
    ASSERT(parametrisation_.get("prune-epsilon", pruneEpsilon_));

    matrixValidate_ = eckit::Resource<bool>("$MIR_MATRIX_VALIDATE", false);
    matrixAssemble_ = parametrisation_.userParametrisation().has("filter");

    std::string nonLinear = "missing-if-heaviest-missing";
    parametrisation_.get("non-linear", nonLinear);
    for (auto& n : eckit::StringTools::split("/", nonLinear)) {
        addNonLinearTreatment(nonlinear::NonLinearFactory::build(n, parametrisation_));
        ASSERT(nonLinear_.back());
    }
}


MethodWeighted::~MethodWeighted() = default;


void MethodWeighted::print(std::ostream& out) const {
    out << "nonLinear[";
    auto sep = "";
    for (auto& n : nonLinear_) {
        out << sep << (*n);
        sep = ",";
    }
    out << "]";

    out << ",cropping=" << cropping_ << ",lsmWeightAdjustment=" << lsmWeightAdjustment_
        << ",pruneEpsilon=" << pruneEpsilon_;
}


bool MethodWeighted::sameAs(const Method& other) const {
    auto sameNonLinearities = [](const std::vector<std::unique_ptr<const nonlinear::NonLinear>>& a,
                                 const std::vector<std::unique_ptr<const nonlinear::NonLinear>>& b) {
        if (a.size() != b.size()) {
            return false;
        }
        for (size_t i = 0; i < a.size(); ++i) {
            if (!a[i]->sameAs(*b[i])) {
                return false;
            }
        }
        return true;
    };

    auto o = dynamic_cast<const MethodWeighted*>(&other);
    return (o != nullptr) && (lsmWeightAdjustment_ == o->lsmWeightAdjustment_) && (pruneEpsilon_ == o->pruneEpsilon_) &&
           (sameNonLinearities(nonLinear_, o->nonLinear_)) &&
           lsm::LandSeaMasks::sameLandSeaMasks(parametrisation_, o->parametrisation_) && cropping_.sameAs(o->cropping_);
}


// Called from MatrixCacheCreator when a matrix in not found in disk cache
void MethodWeighted::createMatrix(context::Context& ctx, const repres::Representation& in,
                                  const repres::Representation& out, WeightMatrix& W, const lsm::LandSeaMasks& masks,
                                  const Cropping& /*cropping*/) const {
    trace::ResourceUsage usage(std::string("MethodWeighted::createMatrix [") + name() + "]", Log::debug());

    computeMatrixWeights(ctx, in, out, W, validateMatrixWeights());

    if (masks.active() && masks.cacheable()) {
        applyMasks(W, masks);
        if (matrixValidate_) {
            W.validate("applyMasks");
        }
    }
}

// This returns a 'const' matrix so we ensure that we don't change it and break the in-memory cache
const WeightMatrix& MethodWeighted::getMatrix(context::Context& ctx, const repres::Representation& in,
                                              const repres::Representation& out) const {
    std::lock_guard<std::mutex> lock(local_mutex);

    auto& log = Log::debug();

    log << "MethodWeighted::getMatrix " << *this << std::endl;
    trace::Timer timer("MethodWeighted::getMatrix");


    double here                   = timer.elapsed();
    const lsm::LandSeaMasks masks = getMasks(in, out);
    log << "MethodWeighted::getMatrix land-sea masks: " << timer.elapsedSeconds(here) << ", "
        << (masks.active() ? "active" : "not active") << std::endl;


    here                             = timer.elapsed();
    const std::string& shortName_in  = in.uniqueName();
    const std::string& shortName_out = out.uniqueName();

    // TODO: add (possibly) missing unique identifiers
    // NOTE: key has to be relatively short, to avoid filesystem "File name too long" errors
    // Check with $getconf -a | grep -i name
    eckit::MD5 hash;
    hash << *this << shortName_in << shortName_out << in.boundingBox() << out.boundingBox();

    std::string version_str;
    auto v = version();
    if (bool(v)) {
        version_str = std::to_string(v) + "/";
    }

    std::string disk_key =
        std::string(name()) + "/" + version_str + shortName_in + "/" + shortName_out + "-" + std::string(hash);
    std::string memory_key = disk_key;

    // Add masks if any
    if (masks.active()) {
        std::string masks_key = "-lsm-" + masks.cacheName();
        memory_key += masks_key;
        if (masks.cacheable()) {
            disk_key += masks_key;
        }
    }


    {
        auto j     = matrix_cache.find(memory_key);
        auto found = j != matrix_cache.end();
        log << "MethodWeighted::getMatrix cache key: " << memory_key << " " << timer.elapsedSeconds(here) << ", "
            << (found ? "found" : "not found") << " in memory cache" << std::endl;
        if (found) {
            const WeightMatrix& mat = *j;
            log << "Using matrix from InMemoryCache " << mat << std::endl;
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
    }
    else {
        createMatrix(ctx, in, out, W, masks, cropping_);
    }

    // If LSM not cacheable to disk, because it is user provided
    // it will be cached in memory nevertheless
    if (masks.active() && !masks.cacheable()) {
        applyMasks(W, masks);
        if (matrixValidate_) {
            W.validate("applyMasks");
        }
    }

    log << "MethodWeighted::getMatrix create weights matrix: " << timer.elapsedSeconds(here) << std::endl;
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


void MethodWeighted::addNonLinearTreatment(const nonlinear::NonLinear* n) {
    ASSERT(n != nullptr);
    nonLinear_.emplace_back(n);
}


void MethodWeighted::setSolver(const solver::Solver* s) {
    ASSERT(s != nullptr);
    solver_.reset(s);
}


void MethodWeighted::setOperandMatricesFromVectors(WeightMatrix::Matrix& A, WeightMatrix::Matrix& B,
                                                   const MIRValuesVector& Avector, const MIRValuesVector& Bvector,
                                                   const double& missingValue, const data::Space& space) const {

    // set input matrix B (from A = W × B)
    // FIXME: remove const_cast once Matrix provides read-only view
    WeightMatrix::Matrix Bwrap(const_cast<double*>(Bvector.data()), Bvector.size(), 1);

    space.linearise(Bwrap, B, missingValue);

    // set output matrix A (from A = W × B)
    // reuses output values vector if handling a column vector, otherwise allocates new matrix
    if (B.cols() == 1) {

        // FIXME: remove const_cast once Matrix provides read-only view
        WeightMatrix::Matrix Awrap(const_cast<double*>(Avector.data()), Avector.size(), 1);
        A.swap(Awrap);
    }
    else {

        WeightMatrix::Matrix Awrap(Avector.size(), B.cols());
        Awrap.setZero();
        A.swap(Awrap);
    }
}


void MethodWeighted::setVectorFromOperandMatrix(const WeightMatrix::Matrix& A, MIRValuesVector& Avector,
                                                const double& missingValue, const data::Space& space) const {

    // set output vector A (from A = W × B)
    // FIXME: remove const_cast once Matrix provides read-only view
    ASSERT(Avector.size() == A.rows());
    WeightMatrix::Matrix Awrap(const_cast<double*>(Avector.data()), Avector.size(), 1);

    space.unlinearise(A, Awrap, missingValue);
}


lsm::LandSeaMasks MethodWeighted::getMasks(const repres::Representation& in, const repres::Representation& out) const {
    return lsm::LandSeaMasks::lookup(parametrisation_, in, out);
}


bool MethodWeighted::validateMatrixWeights() const {
    return true;
}


void MethodWeighted::execute(context::Context& ctx, const repres::Representation& in,
                             const repres::Representation& out) const {


    // Make sure another thread to no evict anything from the cache while we are using it
    auto cacheUse(ctx.statistics().cacheUser(matrix_cache));

    static bool check_stats = eckit::Resource<bool>("mirCheckStats", false);

    trace::Timer timer("MethodWeighted::execute");
    auto& log = Log::debug();
    log << "MethodWeighted::execute" << std::endl;

    // setup sizes & checks
    const size_t npts_inp = in.numberOfPoints();
    const size_t npts_out = out.numberOfPoints();

    const WeightMatrix& W = getMatrix(ctx, in, out);
    ASSERT(W.rows() == npts_out);
    ASSERT(W.cols() == npts_inp);

    std::vector<size_t> forceMissing;  // reserving size unnecessary (not the general case)
    if (!in.isGlobal() || canIntroduceMissingValues()) {
        auto begin = W.begin(0);
        auto end(begin);
        for (size_t r = 0; r < W.rows(); r++) {
            if (begin == (end = W.end(r))) {
                forceMissing.push_back(r);
            }
            begin = end;
        }
    }

    data::MIRField& field     = ctx.field();
    const bool hasMissing     = field.hasMissing() || !forceMissing.empty();
    const double missingValue = hasMissing ? field.missingValue() : std::numeric_limits<double>::quiet_NaN();

    // matrix copy: run-time modifiable matrix is not cacheable
    bool matrixCopy = hasMissing;
    if (!matrixCopy) {
        for (auto& n : nonLinear_) {
            if ((matrixCopy = n->canIntroduceMissingValues())) {
                break;
            }
        }
    }


    for (size_t i = 0; i < field.dimensions(); i++) {

        std::ostringstream os;
        os << "Interpolating field (" << Log::Pretty(npts_inp) << " -> " << Log::Pretty(npts_out) << ")";
        trace::Timer trace(os.str());

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
        WeightMatrix::Matrix A;
        WeightMatrix::Matrix B;
        setOperandMatricesFromVectors(B, A, result, field.values(i), missingValue, sp);
        ASSERT(A.rows() == npts_inp);
        ASSERT(B.rows() == npts_out);


        if (matrixCopy) {
            auto timing(ctx.statistics().matrixTimer());
            WeightMatrix M(W);  // modifiable matrix copy

            for (auto& n : nonLinear_) {
                std::ostringstream str;
                str << *n;
                trace::Timer t(str.str());

                if (n->treatment(A, M, B, field.values(i), missingValue)) {
                    if (matrixValidate_) {
                        M.validate(str.str().c_str());
                    }
                }
            }

            solver_->solve(A, M, B, missingValue);
        }
        else {
            auto timing(ctx.statistics().matrixTimer());
            solver_->solve(A, W, B, missingValue);
        }


        // update field values with interpolation result
        setVectorFromOperandMatrix(B, result, missingValue, sp);

        for (auto& r : forceMissing) {
            result[r] = missingValue;
        }
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


void MethodWeighted::computeMatrixWeights(context::Context& ctx, const repres::Representation& in,
                                          const repres::Representation& out, WeightMatrix& W, bool validate) const {
    auto timing(ctx.statistics().computeMatrixTimer());

    if (in.sameAs(out) && !matrixAssemble_) {
        Log::debug() << "Matrix is identity" << std::endl;
        W.setIdentity(W.rows(), W.cols());
    }
    else {
        trace::Timer timer("Assemble matrix");
        assemble(ctx.statistics(), W, in, out);
        W.cleanup(pruneEpsilon_);
    }

    // matrix validation always happens after creation, because the matrix can/will be cached
    if (validate) {
        W.validate("computeMatrixWeights");
    }
}


void MethodWeighted::applyMasks(WeightMatrix& W, const lsm::LandSeaMasks& masks) const {
    trace::Timer timer("MethodWeighted::applyMasks");
    auto& log = Log::debug();

    log << "MethodWeighted::applyMasks(" << masks << ")" << std::endl;

    ASSERT(masks.active());

    const std::vector<bool>& imask = masks.inputMask();
    const std::vector<bool>& omask = masks.outputMask();

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
        double sum       = 0.;
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
    log << "MethodWeighted: applyMasks corrected " << Log::Pretty(fix) << " out of "
        << Log::Pretty(W.rows(), {"output point"}) << std::endl;
}


void MethodWeighted::hash(eckit::MD5& md5) const {
    md5.add(name());
    md5 << pruneEpsilon_;
    md5 << lsmWeightAdjustment_;
    for (auto& n : nonLinear_) {
        n->hash(md5);
    }

    auto v = version();
    if (v != 0) {
        md5.add(v);
    }
}


int MethodWeighted::version() const {
    return 0;
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
