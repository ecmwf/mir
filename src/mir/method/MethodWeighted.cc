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

#include <algorithm>
#include <fstream>
#include <limits>
#include <sstream>
#include <string>

#include "eckit/config/Resource.h"
#include "eckit/filesystem/PathName.h"
#include "eckit/log/JSON.h"
#include "eckit/types/FloatCompare.h"
#include "eckit/utils/MD5.h"
#include "eckit/utils/StringTools.h"

#include "mir/action/context/Context.h"
#include "mir/caching/InMemoryCache.h"
#include "mir/caching/WeightCache.h"
#include "mir/config/LibMir.h"
#include "mir/data/MIRField.h"
#include "mir/data/MIRFieldStats.h"
#include "mir/data/Space.h"
#include "mir/lsm/LandSeaMasks.h"
#include "mir/method/MatrixCacheCreator.h"
#include "mir/method/nonlinear/NonLinear.h"
#include "mir/method/solver/Multiply.h"
#include "mir/param/DefaultParametrisation.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Representation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"
#include "mir/util/MIRStatistics.h"
#include "mir/util/Mutex.h"
#include "mir/util/Reorder.h"
#include "mir/util/Trace.h"
#include "mir/util/Types.h"


namespace mir::method {


static util::recursive_mutex MUTEX;

constexpr size_t MIR_MATRIX_CACHE_MEMORY_FOOTPRINT = 512 * 1024 * 1024;  // capacity
static caching::InMemoryCache<WeightMatrix> MATRIX_CACHE_MEMORY("mirMatrix", MIR_MATRIX_CACHE_MEMORY_FOOTPRINT, 0,
                                                                "$MIR_MATRIX_CACHE_MEMORY_FOOTPRINT");


// WeightCache is parametrised by 'caching' (it may be disabled for specific fields, eg. unstructured grids)
static caching::WeightCache& matrix_cache_disk(const param::MIRParametrisation& parametrisation) {
    static caching::WeightCache cache(parametrisation);
    return cache;
}


static void matrix_write(const WeightMatrix& mat, const eckit::PathName& path) {
    path.dirName().mkdir(0777);  // ensure directory exists
    mat.save(path);
}


MethodWeighted::MethodWeighted(const param::MIRParametrisation& param) :
    Method(param),
    lsmWeightAdjustment_(param::DefaultParametrisation::instance().get_value<double>("lsm-weight-adjustment", param)),
    pruneEpsilon_(param::DefaultParametrisation::instance().get_value<double>("prune-epsilon", param)),
    poleDisplacement_(
        param::DefaultParametrisation::instance().get_value<double>("pole-displacement-in-degree", param)),
    solver_(new solver::Multiply(param)) {
    ASSERT(lsmWeightAdjustment_ >= 0);
    ASSERT(pruneEpsilon_ >= 0);
    ASSERT(poleDisplacement_ >= 0);

    matrixAssemble_ = parametrisation_.userParametrisation().has("filter");

    std::string nonLinear = "missing-if-heaviest-missing";
    parametrisation_.get("non-linear", nonLinear);
    for (auto& n : eckit::StringTools::split("/", nonLinear)) {
        addNonLinearTreatment(nonlinear::NonLinearFactory::build(n, parametrisation_));
        ASSERT(nonLinear_.back());
    }

    parametrisation_.get("interpolation-matrix", interpolationMatrix_);
}


MethodWeighted::~MethodWeighted() = default;


void MethodWeighted::json(eckit::JSON& j) const {
    // NOTE: only output non-default (uncluttered) and configurable (hence, usable) options
    j << "type" << type();

    if (!nonLinear_.empty()) {
        j << "non-linear";
        j.startList();
        for (const auto& n : nonLinear_) {
            j << n->name();
        }
        j.endList();

        for (const auto& n : nonLinear_) {
            n->json(j);
        }
    }

    param::DefaultParametrisation::instance().json(j, "pole-displacement-in-degree", poleDisplacement_);
    param::DefaultParametrisation::instance().json(j, "prune-epsilon", pruneEpsilon_);
    param::DefaultParametrisation::instance().json(j, "lsm-weight-adjustment", lsmWeightAdjustment_);
}


void MethodWeighted::print(std::ostream& out) const {
    out << "nonLinear[";
    const auto* sep = "";
    for (const auto& n : nonLinear_) {
        out << sep << (*n);
        sep = ",";
    }
    out << "]";

    if (!reorderRows_.empty()) {
        out << ",reorderRows=" << reorderRows_;
    }

    if (!reorderCols_.empty()) {
        out << ",reorderCols=" << reorderCols_;
    }

    out << ",solver=" << *solver_;
    out << ",cropping=" << cropping_;
    out << ",lsmWeightAdjustment=" << lsmWeightAdjustment_;
    out << ",pruneEpsilon=" << pruneEpsilon_;
    out << ",poleDisplacement=" << poleDisplacement_;
}


bool MethodWeighted::sameAs(const Method& other) const {
    auto sameNonLinearities = [](const auto& a, const auto& b) {
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

    const auto* o = dynamic_cast<const MethodWeighted*>(&other);
    return (o != nullptr) && (lsmWeightAdjustment_ == o->lsmWeightAdjustment_) && (pruneEpsilon_ == o->pruneEpsilon_) &&
           Latitude(poleDisplacement_) == Latitude(o->poleDisplacement_) &&
           (sameNonLinearities(nonLinear_, o->nonLinear_)) && solver().sameAs(o->solver()) &&
           lsm::LandSeaMasks::sameLandSeaMasks(parametrisation_, o->parametrisation_) && cropping_.sameAs(o->cropping_);
}


// Called from MatrixCacheCreator when a matrix in not found in disk cache
void MethodWeighted::createMatrix(context::Context& ctx, const repres::Representation& in,
                                  const repres::Representation& out, WeightMatrix& W, const lsm::LandSeaMasks& masks,
                                  const Cropping& /*cropping*/) const {
    trace::ResourceUsage usage(std::string("MethodWeighted::createMatrix [") + type() + "]");
    const auto checks = validateMatrixWeights();

    // matrix validation always happens after creation, because the matrix can/will be cached
    computeMatrixWeights(ctx, in, out, W);
    W.validate("computeMatrixWeights", checks);

    if (masks.active() && masks.cacheable()) {
        applyMasks(W, masks);
        W.validate("applyMasks", checks);
    }

    bool bitmask;
    parametrisation_.get("imm", bitmask);
    if (bitmask){
        std::vector<bool> vec_bitmask;
        parametrisation_.get("imm-mask", vec_bitmask);
        applyIMM(W,vec_bitmask);
        if (matrixValidate_) {
            W.validate("applyMasks");
        }
    }

}


MethodWeighted::CacheKeys MethodWeighted::getDiskAndMemoryCacheKeys(const repres::Representation& in,
                                                                    const repres::Representation& out,
                                                                    const lsm::LandSeaMasks& masks) const {
    const std::string& shortName_in  = in.uniqueName();
    const std::string& shortName_out = out.uniqueName();

    // TODO: add (possibly) missing unique identifiers
    // NOTE: key has to be relatively short, to avoid filesystem "File name too long" errors
    // Check with $getconf -a | grep -i name
    eckit::MD5 hash;
    hash << *this << shortName_in << shortName_out << in.boundingBox() << out.boundingBox();


    std::string version_str;
    if (auto v = version(); v != 0) {
        version_str = std::to_string(v) + "/";
    }

    auto disk_key = std::string{type()} + "/" + version_str + shortName_in + "/" + shortName_out + "-" + hash.digest();
    auto memory_key = disk_key;

    // Add masks if any
    if (masks.active()) {
        std::string masks_key = "-lsm-" + masks.cacheName();
        memory_key += masks_key;
        if (masks.cacheable()) {
            disk_key += masks_key;
        }
    }

    bool bitmask;
    parametrisation_.get("imm", bitmask);
    if (bitmask){
        std::string missing_mask_key;
        parametrisation_.get("imm-name", missing_mask_key);
        memory_key += "_" + missing_mask_key;
        disk_key += "_" + missing_mask_key;
     }
    return {disk_key, memory_key};
}


// This returns a 'const' matrix so we ensure that we don't change it and break the in-memory cache
const WeightMatrix& MethodWeighted::getMatrix(context::Context& ctx, const repres::Representation& in,
                                              const repres::Representation& out) const {
    util::lock_guard<util::recursive_mutex> lock(MUTEX);
    auto& log = Log::debug();

    log << "MethodWeighted::getMatrix " << *this << std::endl;
    trace::Timer timer("MethodWeighted::getMatrix");

    double here      = timer.elapsed();
    const auto masks = getMasks(in, out);
    log << "MethodWeighted::getMatrix land-sea masks: " << timer.elapsedSeconds(here) << ", "
        << (masks.active() ? "active" : "not active") << std::endl;

    const auto [disk_key, memory_key] = getDiskAndMemoryCacheKeys(in, out, masks);
    ASSERT(!disk_key.empty() && !memory_key.empty());

    if (auto* j = MATRIX_CACHE_MEMORY.find(memory_key); j != MATRIX_CACHE_MEMORY.end()) {
        const auto& mat = *j;
        log << "MethodWeighted::getMatrix cache key: " << memory_key << " " << timer.elapsedSeconds(here)
            << ", found in memory cache (" << mat << ")" << std::endl;

        if (!interpolationMatrix_.empty()) {
            matrix_write(mat, interpolationMatrix_);
        }

        return mat;
    }

    log << "MethodWeighted::getMatrix cache key: " << memory_key << " " << timer.elapsedSeconds(here)
        << ", not found in memory cache" << std::endl;


    // calculate weights matrix, apply mask if necessary
    here = timer.elapsed();
    WeightMatrix W(out.numberOfPoints(), in.numberOfPoints());

    eckit::PathName cacheFile;

    bool caching = LibMir::caching();
    parametrisation_.get("caching", caching);

    if (const std::string ext = caching::WeightCacheTraits::extension(); eckit::StringTools::endsWith(disk_key, ext)) {
        caching::WeightCacheTraits::load(matrix_cache_disk(parametrisation_), W, disk_key);
        cacheFile = disk_key;
    }
    else if (caching) {
        MatrixCacheCreator creator(*this, ctx, in, out, masks, cropping_);
        cacheFile = matrix_cache_disk(parametrisation_).getOrCreate(disk_key, creator, W);
    }
    else {
        createMatrix(ctx, in, out, W, masks, cropping_);
    }

    // If LSM not cacheable to disk, because it is user provided
    // it will be cached in memory nevertheless
    if (masks.active() && !masks.cacheable()) {
        applyMasks(W, masks);
        W.validate("applyMasks", validateMatrixWeights());
    }

    if (!interpolationMatrix_.empty()) {
        matrix_write(W, interpolationMatrix_);
    }

    log << "MethodWeighted::getMatrix create weights matrix: " << timer.elapsedSeconds(here) << std::endl;
    log << "MethodWeighted::getMatrix matrix W " << W << std::endl;

    // insert matrix in the in-memory cache and update memory footprint

    WeightMatrix& w = MATRIX_CACHE_MEMORY[memory_key];
    W.swap(w);

    size_t footprint = w.footprint();
    caching::InMemoryCacheUsage usage(w.inSharedMemory() ? 0 : footprint, w.inSharedMemory() ? footprint : 0);

    log << "Matrix footprint " << w.owner() << " " << usage << " W -> " << W.owner() << std::endl;

    if (std::string filename; parametrisation_.get("dump-weights-info", filename)) {
        std::ofstream file(filename);
        eckit::JSON j(file);
        j.startObject();
        j << "input" << in;
        j << "output" << out;
        j << "interpolation";
        Method::json(j, true);

        j << "matrix";
        j.startObject();
        j << "engine" << LibMir::instance().name();
        j << "version" << caching::WeightCache::version();
        j << "shape";
        j.startList();
        j << w.rows();
        j << w.cols();
        j.endList();
        j << "nnz" << w.nonZeros();
        j << "cache_file" << cacheFile;
        j.endObject();

        j.endObject();
    }

    MATRIX_CACHE_MEMORY.footprint(memory_key, usage);
    return w;
}


const solver::Solver& MethodWeighted::solver() const {
    ASSERT(solver_);
    return *solver_;
}


void MethodWeighted::addNonLinearTreatment(const nonlinear::NonLinear* n) {
    ASSERT(n != nullptr);
    nonLinear_.emplace_back(n);
}


void MethodWeighted::setSolver(const solver::Solver* s) {
    ASSERT(s != nullptr);
    solver_.reset(s);
}


void MethodWeighted::setOperandMatricesFromVectors(DenseMatrix& A, DenseMatrix& B, const MIRValuesVector& Avector,
                                                   const MIRValuesVector& Bvector, const double& missingValue,
                                                   const data::Space& space) const {

    // set input matrix B (from A = W × B)
    // FIXME: remove const_cast once Matrix provides read-only view
    DenseMatrix Bwrap(const_cast<double*>(Bvector.data()), Bvector.size(), 1);

    space.linearise(Bwrap, B, missingValue);

    // set output matrix A (from A = W × B)
    // reuses output values vector if handling a column vector, otherwise allocates new matrix
    if (B.cols() == 1) {

        // FIXME: remove const_cast once Matrix provides read-only view
        DenseMatrix Awrap(const_cast<double*>(Avector.data()), Avector.size(), 1);
        A.swap(Awrap);
    }
    else {

        DenseMatrix Awrap(Avector.size(), B.cols());
        Awrap.setZero();
        A.swap(Awrap);
    }
}


void MethodWeighted::setVectorFromOperandMatrix(const DenseMatrix& A, MIRValuesVector& Avector,
                                                const double& missingValue, const data::Space& space) const {

    // set output vector A (from A = W × B)
    // FIXME: remove const_cast once Matrix provides read-only view
    ASSERT(Avector.size() == A.rows());
    DenseMatrix Awrap(const_cast<double*>(Avector.data()), Avector.size(), 1);

    space.unlinearise(A, Awrap, missingValue);
}


lsm::LandSeaMasks MethodWeighted::getMasks(const repres::Representation& in, const repres::Representation& out) const {
    return lsm::LandSeaMasks::lookup(parametrisation_, in, out);
}


WeightMatrix::Check MethodWeighted::validateMatrixWeights() const {
    return {};
}


void MethodWeighted::execute(context::Context& ctx, const repres::Representation& in,
                             const repres::Representation& out) const {


    // Make sure another thread to no evict anything from the cache while we are using it
    auto cacheUse(ctx.statistics().cacheUser(MATRIX_CACHE_MEMORY));

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
    {
        auto begin = W.begin(0);
        auto end(begin);
        for (size_t r = 0; r < W.rows(); r++) {
            if (begin == (end = W.end(r))) {
                forceMissing.push_back(r);
            }
            begin = end;
        }
    }

    // ensure unique missingValue on no input missing values
    data::MIRField& field = ctx.field();
    const bool hasMissing = field.hasMissing();
    if (!hasMissing) {
        field.missingValue(std::numeric_limits<double>::lowest());
    }

    const double missingValue = field.missingValue();

    // matrix copy: run-time modifiable matrix is not cacheable
    bool imm_active;
    parametrisation_.get("imm", imm_active);
    bool matrixCopy = false;
    if (!imm_active) {
    matrixCopy = std::any_of(nonLinear_.begin(), nonLinear_.end(),
                                        [&field](const std::unique_ptr<const nonlinear::NonLinear>& n) {
                                            return n->modifiesMatrix(field.hasMissing());
                                        });
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
        parametrisation_.get("vector-space", space);
        const data::Space& sp = data::SpaceChooser::lookup(space);

        MIRValuesVector result(npts_out);  // field.update() takes ownership with std::swap()
        DenseMatrix A;
        DenseMatrix B;
        setOperandMatricesFromVectors(B, A, result, field.values(i), missingValue, sp);
        ASSERT(A.rows() == npts_inp);
        ASSERT(B.rows() == npts_out);


        if (matrixCopy) {
            auto timing(ctx.statistics().matrixTimer());
            WeightMatrix M(W);  // modifiable matrix copy

            for (const auto& n : nonLinear_) {
                std::ostringstream str;
                str << *n;
                trace::Timer t(str.str());

                if (n->treatment(A, M, B, field.values(i), missingValue)) {
                    M.validate(str.str().c_str(), validateMatrixWeights());
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
        field.update(result, i, hasMissing || !forceMissing.empty());


        if (check_stats) {
            // compute some statistics on the result
            auto ostats = field.statistics(i);
            log << "Input field statistics:  " << istats << "\n"
                << "Output field statistics: " << ostats << std::endl;
        }
    }
}


void MethodWeighted::computeMatrixWeights(context::Context& ctx, const repres::Representation& in,
                                          const repres::Representation& out, WeightMatrix& W) const {
    auto timing(ctx.statistics().computeMatrixTimer());

    if (in.sameAs(out) && !matrixAssemble_) {
        Log::debug() << "Matrix is identity" << std::endl;
        W.setIdentity(W.rows(), W.cols());
    }
    else {
        trace::Timer timer("Assemble matrix");
        assemble(ctx.statistics(), W, in, out);
        W.cleanup(pruneEpsilon_);

        if (!reorderRows_.empty() || !reorderCols_.empty()) {
            using util::Reorder;

            auto rows =
                std::unique_ptr<Reorder>(Reorder::build(reorderRows_.empty() ? "identity" : reorderRows_, W.rows()))
                    ->reorder();
            ASSERT(rows.size() == out.numberOfPoints());

            auto cols =
                std::unique_ptr<Reorder>(Reorder::build(reorderCols_.empty() ? "identity" : reorderCols_, W.cols()))
                    ->reorder();
            ASSERT(cols.size() == in.numberOfPoints());

            // expand triplets, renumbering directly
            std::vector<eckit::linalg::Triplet> trips;
            trips.reserve(W.nonZeros());

            for (auto i = W.begin(), end = W.end(); i != end; ++i) {
                trips.emplace_back(rows.at(i.row()), cols.at(i.col()), *i);
            }

            // compress triplets, replace matrix
            std::sort(trips.begin(), trips.end());

            eckit::linalg::SparseMatrix w(W.rows(), W.cols(), trips);
            W.swap(w);
        }
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

void MethodWeighted::applyIMM(WeightMatrix& W, const std::vector<bool>& imask) const {
   trace::Timer timer("MethodWeighted::applyIMM");
auto& log = Log::debug();

log << "MethodWeighted::applyIMM" << std::endl;
log << "mask size=" << imask.size() << " == #cols=" << W.cols() << std::endl;

// Ensure that the mask size matches the number of columns in W
ASSERT(imask.size() == W.cols());

auto* data = const_cast<WeightMatrix::Scalar*>(W.data());
auto* outer = W.outer();
auto* inner = W.inner();
size_t fix = 0;

// Iterate over each row
for (WeightMatrix::Size r = 0; r < W.rows(); ++r) {
    WeightMatrix::Size row_start = outer[r];
    WeightMatrix::Size row_end = outer[r + 1];

    size_t i_missing = row_start;
    size_t N_missing = 0;
    size_t N_entries = row_end - row_start;
    double sum = 0.0;
    double heaviest = -1.0;
    bool heaviest_is_missing = false;

    // Iterate over the entries in the current row
    for (WeightMatrix::Size i = row_start; i < row_end; ++i) {
        const bool miss = !imask[inner[i]];

        if (miss) {
            ++N_missing;
            i_missing = i;
        } else {
            sum += data[i];
        }

        if (heaviest < data[i]) {
            heaviest = data[i];
            heaviest_is_missing = miss;
        }
    }

    // Weights redistribution: zero-weight all missing values, linear re-weighting for the others
    if (N_missing > 0) {
        ++fix;
        if (N_missing == N_entries || heaviest_is_missing || eckit::types::is_approximately_equal(sum, 0.0)) {
            // All values are missing or the heaviest is missing; set only i_missing to 1
            for (WeightMatrix::Size i = row_start; i < row_end; ++i) {
                data[i] = (i == i_missing) ? 1.0 : 0.0;
            }
        } else {
            // Scale non-missing entries so they sum to 1
            const double factor = 1.0 / sum;
            for (WeightMatrix::Size i = row_start; i < row_end; ++i) {
                const bool miss = !imask[inner[i]];
                data[i] = miss ? 0.0 : (factor * data[i]);
            }
        }
    }
}

// Log the number of corrections made
log << "MethodWeighted: applyIMM corrected " << Log::Pretty(fix) << " out of "
    << Log::Pretty(W.rows(), {"Weight matrix rows"}) << std::endl;
}     

void MethodWeighted::hash(eckit::MD5& md5) const {
    md5.add(type());
    md5 << pruneEpsilon_;
    md5 << poleDisplacement_;
    md5 << lsmWeightAdjustment_;

    for (const auto& n : nonLinear_) {
        n->hash(md5);
    }

    if (auto v = version(); v != 0) {
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


}  // namespace mir::method
