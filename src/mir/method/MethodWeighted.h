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


#ifndef mir_method_MethodWeighted_H
#define mir_method_MethodWeighted_H

#include "mir/caching/WeightCache.h"
#include "mir/method/Method.h"
#include "mir/method/WeightMatrix.h"


namespace atlas {
namespace grid {
class Grid;
}
namespace mesh {
class Mesh;
}
}
namespace mir {
namespace lsm {
class LandSeaMasks;
}
namespace util {
class MIRStatistics;
}
namespace data {
class MIRField;
}
}


namespace mir {
namespace method {

class GridSpace;

//----------------------------------------------------------------------------------------------------------------------

class MethodWeighted : public Method {

public:

    explicit MethodWeighted(const param::MIRParametrisation &);

    virtual ~MethodWeighted();

    virtual void execute(context::Context &ctx, const atlas::grid::Grid &in, const atlas::grid::Grid &out) const;

    virtual void hash(eckit::MD5&) const;

    atlas::mesh::Mesh& generateMeshAndCache(const atlas::grid::Grid& grid) const;

    virtual void generateMesh(const atlas::grid::Grid& g, atlas::mesh::Mesh& mesh) const;

protected:

private:

    double lsmWeightAdjustement_;

    virtual const char *name() const = 0;

    virtual void assemble(context::Context& ctx, WeightMatrix &W, const GridSpace& in, const GridSpace& out) const = 0;

    /// Update interpolation weigths matrix to account for missing values
    WeightMatrix applyMissingValues(const WeightMatrix &W, const std::vector<bool>& fieldMissingValues) const;

    /// Update interpolation weigths matrix to account for field masked values
    virtual void applyMasks(WeightMatrix &W, const lsm::LandSeaMasks &, util::MIRStatistics& statistics) const;

    virtual const WeightMatrix &getMatrix(context::Context& ctx, const atlas::grid::Grid &in, const atlas::grid::Grid &out) const;

    /// Get interpolation operand matrices, from A = W × B
    virtual void setOperandMatricesFromVectors(WeightMatrix::Matrix& A, WeightMatrix::Matrix& B, const std::vector<double>& Avector, const std::vector<double>& Bvector, const double& missingValue) const;

    /// Get interpolation operand matrices, from A = W × B
    virtual void setVectorFromOperandMatrix(const WeightMatrix::Matrix& A, std::vector<double>& Avector, const double& missingValue) const;

    virtual lsm::LandSeaMasks getMasks(context::Context& ctx, const atlas::grid::Grid &in, const atlas::grid::Grid &out) const;

    void computeMatrixWeights(context::Context& ctx, const atlas::grid::Grid &in, const atlas::grid::Grid &out, WeightMatrix& W) const;

    void createMatrix(context::Context& ctx, const atlas::grid::Grid &in, const atlas::grid::Grid &out, WeightMatrix& W, const lsm::LandSeaMasks& masks) const;

};

//----------------------------------------------------------------------------------------------------------------------

}  // namespace method
}  // namespace mir

#endif

