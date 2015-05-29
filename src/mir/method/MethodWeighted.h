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

#include "mir/method/Method.h"
#include "mir/caching/WeightCache.h"
#include "mir/method/WeightMatrix.h"


namespace atlas {
class Grid;
}


namespace mir {
namespace lsm {
class LandSeaMasks;
}
}


namespace mir {
namespace method {

//----------------------------------------------------------------------------------------------------------------------

class MethodWeighted : public Method {

  public:

    explicit MethodWeighted(const param::MIRParametrisation &);

    virtual ~MethodWeighted();

    virtual void execute(data::MIRField &field, const atlas::Grid &in, const atlas::Grid &out) const;

    virtual void hash( eckit::MD5 & ) const;

  protected:

    virtual const char *name() const = 0;

    virtual void assemble(WeightMatrix &W, const atlas::Grid &in, const atlas::Grid &out) const = 0;

    /// Update interpolation weigths matrix to account for missing values
    WeightMatrix applyMissingValues(const WeightMatrix &W, data::MIRField &field, size_t which) const;

    /// Update interpolation weigths matrix to account for field masked values
    virtual void applyMasks(WeightMatrix &W, const lsm::LandSeaMasks &) const;


    virtual const WeightMatrix &getMatrix(const atlas::Grid &in, const atlas::Grid &out) const;

    virtual lsm::LandSeaMasks getMasks(const atlas::Grid &in, const atlas::Grid &out) const;

  private:

    void computeMatrixWeights(const atlas::Grid &in, const atlas::Grid &out, WeightMatrix &W) const;

    /// Confirm the matrix weights are correct.
    /// @note this is intended for development only
    /// @return number of rows with issues: sum(W(i,:))!=(0,1), or W(i,j)<0, or W(i,j)>1
    void checkMatrixWeights(const WeightMatrix &W, const char *when, const atlas::Grid &in, const atlas::Grid &out) const;
    void cleanupMatrix(WeightMatrix &W) const;


};

//----------------------------------------------------------------------------------------------------------------------

}  // namespace method
}  // namespace mir

#endif

