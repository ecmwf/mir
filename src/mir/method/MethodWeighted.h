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


#ifndef mir_method_MethodWeighted_H
#define mir_method_MethodWeighted_H

#include "mir/caching/WeightCache.h"
#include "mir/method/Method.h"
#include "mir/method/WeightMatrix.h"


namespace atlas {
class Grid;
class Mesh;
}
namespace mir {
namespace lsm {
class LandSeaMasks;
}
}


namespace mir {
namespace method {


class MethodWeighted : public Method {

public:

    explicit MethodWeighted(const param::MIRParametrisation&);

    virtual ~MethodWeighted();

    virtual void execute(context::Context&, const MIRGrid& in, const MIRGrid& out) const;

    virtual void hash(eckit::MD5&) const;

    atlas::Mesh& generateMeshAndCache(const atlas::Grid&) const;

    virtual void generateMesh(const atlas::Grid&, atlas::Mesh&) const;

protected:

    virtual const WeightMatrix& getMatrix(context::Context&, const MIRGrid& in, const MIRGrid& out) const;

private:

    virtual const char *name() const = 0;

    virtual void assemble(WeightMatrix&, const MIRGrid& in, const MIRGrid& out) const = 0;

    /// Update interpolation weigths matrix to account for missing values
    void applyMissingValues(const WeightMatrix& W, const std::vector<double>& values, const double& missingValue, WeightMatrix& MW) const;

    /// Update interpolation weigths matrix to account for field masked values
    virtual void applyMasks(WeightMatrix&, const lsm::LandSeaMasks&) const;

    /// Get interpolation operand matrices, from A = W × B
    virtual void setOperandMatricesFromVectors(WeightMatrix::Matrix& A, WeightMatrix::Matrix& B, const std::vector<double>& Avector, const std::vector<double>& Bvector, const double& missingValue) const;

    /// Get interpolation operand matrices, from A = W × B
    virtual void setVectorFromOperandMatrix(const WeightMatrix::Matrix& A, std::vector<double>& Avector, const double& missingValue) const;

    virtual lsm::LandSeaMasks getMasks(const atlas::Grid& in, const atlas::Grid& out) const;

    void computeMatrixWeights(context::Context&, const MIRGrid& in, const MIRGrid& out, WeightMatrix&) const;

    void createMatrix(context::Context&, const MIRGrid& in, const MIRGrid& out, WeightMatrix&, const lsm::LandSeaMasks&) const;

private:

    double lsmWeightAdjustement_;

};


}  // namespace method
}  // namespace mir


#endif

