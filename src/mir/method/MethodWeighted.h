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

    virtual void hash( eckit::MD5& ) const;

  protected:

    virtual void assemble(WeightMatrix &W, const atlas::Grid &in, const atlas::Grid &out) const = 0;

    /// Update interpolation weigths matrix to account for missing values
    WeightMatrix applyMissingValues(const WeightMatrix &W, data::MIRField &field, size_t which) const;

    /// Update interpolation weigths matrix to account for field masked values

    void applyMasks(WeightMatrix &W, const lsm::LandSeaMasks &) const;

    const WeightMatrix &getMatrix(const atlas::Grid &in, const atlas::Grid &out) const;

  private:

    virtual const char *name() const = 0;

    void computeWeights(const atlas::Grid &in, const atlas::Grid &out, WeightMatrix &W) const;

    template< typename _UnaryOperation >
    std::vector< bool > computeFieldMask(const _UnaryOperation& op, const data::MIRField& field, size_t which) const;

};

//----------------------------------------------------------------------------------------------------------------------

}  // namespace method
}  // namespace mir

#endif

