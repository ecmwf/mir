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


#include "mir/method/Matrix.h"

#include "eckit/log/JSON.h"
#include "eckit/utils/MD5.h"
#include "eckit/utils/StringTools.h"

#include "mir/action/context/Context.h"
#include "mir/caching/InMemoryCacheUsage.h"
#include "mir/caching/matrix/MatrixLoader.h"
#include "mir/config/LibMir.h"
#include "mir/data/MIRField.h"
#include "mir/data/Space.h"
#include "mir/method/MethodWeighted.h"
#include "mir/method/nonlinear/NonLinear.h"
#include "mir/method/solver/Multiply.h"
#include "mir/method/solver/Statistics.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Representation.h"
#include "mir/stats/Field.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"
#include "mir/util/MIRStatistics.h"
#include "mir/util/Trace.h"


namespace mir::method {


static const MethodBuilder<Matrix> __builder("matrix");


Matrix::Matrix(const param::MIRParametrisation& param) : Method(param) {
    std::string matrix;
    if (!parametrisation_.get("interpolation-matrix", matrix)) {
        throw exception::UserError("Matrix: option interpolation-matrix missing");
    }

    matrix_ = matrix;
    if (!matrix_.exists()) {
        throw exception::UserError("Matrix: matrix path does not exist '" + matrix + "'");
    }

    std::string nonLinear = "missing-if-heaviest-missing";
    parametrisation_.get("non-linear", nonLinear);
    for (auto& n : eckit::StringTools::split("/", nonLinear)) {
        nonLinear_.emplace_back(nonlinear::NonLinearFactory::build(n, parametrisation_));
        ASSERT(nonLinear_.back());
    }

    if (std::string stats = "maximum"; parametrisation_.get("interpolation-statistics", stats)) {
        solver_ =
            std::make_unique<solver::Statistics>(parametrisation_, stats::FieldFactory::build(stats, parametrisation_));
    }
    else {
        solver_ = std::make_unique<solver::Multiply>(parametrisation_);
    }
    ASSERT(solver_);

    matrix_loader_ = LibMir::cacheLoader(LibMir::cache_loader::MATRIX);
    parametrisation_.get("matrix-loader", matrix_loader_);

    parametrisation_.get("vector-space", vector_space_);
}


void Matrix::json(eckit::JSON& j) const {
    j.startObject();

    j << "type" << "matrix";
    j << "matrix" << matrix_;
    j << "solver" << *solver_;
    j << "nonLinear";
    j.startList();
    for (const auto& n : nonLinear_) {
        j << *n;
    }
    j.endList();

    j.endObject();
}


void Matrix::execute(context::Context& ctx, const repres::Representation& in, const repres::Representation& out) const {
    trace::Timer timer("Matrix::execute");

    auto& log = Log::debug();
    log << "Matrix::execute (input: " << in << ", output: " << out << ", matrix: " << matrix_ << ")" << std::endl;

    if (!matrix_.exists()) {
        throw exception::UserError("Matrix: file not found: '" + matrix_.realName() + "'");
    }


    // get matrix from in-memory cache, or insert an empty entry
    auto& cache = MethodWeighted::matrix_cache();

    auto key = matrix_.realName().asString();
    auto& W  = cache[key];

    if (W.empty()) {
        log << "Load matrix from file " << matrix_loader_ << std::endl;
        WeightMatrix M(caching::matrix::MatrixLoaderFactory::build(matrix_loader_, matrix_));
        W.swap(M);

        // update memory footprint
        auto shared = W.inSharedMemory();
        caching::InMemoryCacheUsage usage(shared ? 0 : W.footprint(), shared ? W.footprint() : 0);
        cache.footprint(key, usage);

        log << "Matrix footprint " << W.owner() << " " << usage << " W -> " << W.owner() << std::endl;
    }
    else {
        log << "Using matrix from InMemoryCache" << std::endl;
    }

    log << "Matrix: " << W << std::endl;
    ASSERT_NONEMPTY_INTERPOLATION("Matrix", !W.empty());


    // ensure unique missingValue on no input missing values
    auto& field           = ctx.field();
    const bool hasMissing = field.hasMissing();
    if (!hasMissing) {
        field.missingValue(std::numeric_limits<double>::lowest());
    }

    const double missingValue = field.missingValue();


    // check if matrix copy is required: run-time modifiable matrix is not cacheable
    const bool matrixCopy = std::any_of(nonLinear_.begin(), nonLinear_.end(),
                                        [&field](const std::unique_ptr<const nonlinear::NonLinear>& n) {
                                            return n->modifiesMatrix(field.hasMissing());
                                        });


    // check if forcing of missing values is required
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


    // solve

    auto npts_inp = in.numberOfPoints();
    auto npts_out = out.numberOfPoints();

    const auto& sp = data::SpaceChooser::lookup(vector_space_);

    for (size_t i = 0; i < field.dimensions(); i++) {
        MIRValuesVector result(npts_out);  // field.update() takes ownership with std::swap()

        WeightMatrix::Matrix A;
        WeightMatrix::Matrix B;
        MethodWeighted::set_operand_matrices_from_vectors(B, A, result, field.values(i), missingValue, sp);
        ASSERT(A.rows() == npts_inp);
        ASSERT(B.rows() == npts_out);


        // modify matrix (as a copy) if there are non-linear treatments
        if (matrixCopy) {
            auto timing(ctx.statistics().matrixTimer());
            WeightMatrix M(W);  // modifiable matrix copy

            for (const auto& n : nonLinear_) {
                n->treatment(A, M, B, field.values(i), missingValue);
            }

            solver_->solve(A, M, B, missingValue);
        }
        else {
            auto timing(ctx.statistics().matrixTimer());
            solver_->solve(A, W, B, missingValue);
        }


        // update field values with interpolation result
        MethodWeighted::set_vector_from_operand_matrix(B, result, missingValue, sp);

        for (auto& r : forceMissing) {
            result[r] = missingValue;
        }

        field.update(result, i, hasMissing || !forceMissing.empty());
    }
}


void Matrix::hash(eckit::MD5& h) const {
    h << matrix_.asString();

    for (const auto& n : nonLinear_) {
        n->hash(h);
    }

    solver_->hash(h);

    if (auto v = version(); v != 0) {
        h.add(v);
    }
}


bool Matrix::sameAs(const Method& other) const {
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

    const auto* o = dynamic_cast<const Matrix*>(&other);
    return (o != nullptr) && matrix_.sameAs(o->matrix_) && sameNonLinearities(nonLinear_, o->nonLinear_) &&
           solver_->sameAs(*o->solver_);
}


void Matrix::print(std::ostream& out) const {
    out << "Matrix["
        << "InterpolationMatrix=" << matrix_ << ",Solver=" << (*solver_) << ",nonLinear[";
    const auto* sep = "";
    for (const auto& n : nonLinear_) {
        out << sep << (*n);
        sep = ",";
    }
    out << "]"
        << "]";
}


void Matrix::setCropping(const util::BoundingBox&) {
    NOTIMP;
}


const util::BoundingBox& Matrix::getCropping() const {
    NOTIMP;
}


}  // namespace mir::method
