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


#pragma once

#include <memory>
#include <utility>
#include <vector>

#include "eckit/linalg/Matrix.h"

#include "mir/method/Cropping.h"
#include "mir/method/Method.h"
#include "mir/method/WeightMatrix.h"


namespace eckit {
class JSON;
}

namespace mir {
namespace data {
class Space;
}
namespace lsm {
class LandSeaMasks;
}
namespace method {
namespace nonlinear {
class NonLinear;
}
namespace solver {
class Solver;
}
}  // namespace method
namespace reorder {
class Reorder;
}
namespace repres {
class Representation;
}
namespace util {
class MIRStatistics;
}
}  // namespace mir


namespace mir::method {


using DenseMatrix = eckit::linalg::Matrix;


class MethodWeighted : public Method {
public:
    // -- Types

    using CacheKeys = std::pair<std::string, std::string>;

    // -- Constructors

    explicit MethodWeighted(const param::MIRParametrisation&);

    // -- Destructor

    ~MethodWeighted() override;

    // -- Methods

    void hash(eckit::MD5&) const override;

    int version() const override;

    const WeightMatrix& getMatrix(context::Context&, const repres::Representation& in,
                                  const repres::Representation& out) const;

protected:
    // -- Methods

    virtual void json(eckit::JSON&) const = 0;
    virtual const char* name() const      = 0;
    const solver::Solver& solver() const;
    void addNonLinearTreatment(const nonlinear::NonLinear*);
    void setSolver(const solver::Solver*);
    void setReorderRows(reorder::Reorder*);
    void setReorderCols(reorder::Reorder*);
    double poleDisplacement() const { return poleDisplacement_; }

    // -- Overridden methods

    // From Method
    bool sameAs(const Method&) const override = 0;
    void print(std::ostream&) const override  = 0;

private:
    // -- Members

    double lsmWeightAdjustment_;
    double pruneEpsilon_;
    double poleDisplacement_;
    Cropping cropping_;
    std::string interpolationMatrix_;

    std::vector<std::unique_ptr<const nonlinear::NonLinear>> nonLinear_;
    std::unique_ptr<const solver::Solver> solver_;
    std::unique_ptr<const reorder::Reorder> reorderRows_;
    std::unique_ptr<const reorder::Reorder> reorderCols_;

    bool matrixAssemble_;

    // -- Methods

    virtual void assemble(util::MIRStatistics&, WeightMatrix&, const repres::Representation& in,
                          const repres::Representation& out) const = 0;

    virtual CacheKeys getDiskAndMemoryCacheKeys(const repres::Representation& in, const repres::Representation& out,
                                                const lsm::LandSeaMasks&) const;

    virtual void applyMasks(WeightMatrix&, const lsm::LandSeaMasks&) const;
    virtual lsm::LandSeaMasks getMasks(const repres::Representation& in, const repres::Representation& out) const;
    virtual WeightMatrix::Check validateMatrixWeights() const;

    void computeMatrixWeights(context::Context&, const repres::Representation& in, const repres::Representation& out,
                              WeightMatrix&) const;
    void createMatrix(context::Context&, const repres::Representation& in, const repres::Representation& out,
                      WeightMatrix&, const lsm::LandSeaMasks&, const Cropping&) const;

    /// Get interpolation operand matrices, from A = W B
    virtual void setOperandMatricesFromVectors(DenseMatrix& A, DenseMatrix& B, const MIRValuesVector& Avector,
                                               const MIRValuesVector& Bvector, const double& missingValue,
                                               const data::Space&) const;

    /// Get interpolation operand matrices, from A = W B
    virtual void setVectorFromOperandMatrix(const DenseMatrix& A, MIRValuesVector& Avector, const double& missingValue,
                                            const data::Space&) const;

    // -- Overridden methods

    // From Method
    void execute(context::Context&, const repres::Representation& in, const repres::Representation& out) const override;
    bool canCrop() const override;
    void setCropping(const util::BoundingBox&) override;
    bool hasCropping() const override;
    const util::BoundingBox& getCropping() const override;

    // -- Friends

    friend class MatrixCacheCreator;

    friend eckit::JSON& operator<<(eckit::JSON& s, const MethodWeighted& o) {
        o.json(s);
        return s;
    }
};


}  // namespace mir::method
