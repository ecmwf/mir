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
namespace grid {
class Grid;
}
}


namespace mir {
namespace lsm {
class LandSeaMasks;
}

namespace util {
class MIRStatistics;
}

}


namespace mir {
namespace method {

//----------------------------------------------------------------------------------------------------------------------

class MethodWeighted : public Method {

  public:

    explicit MethodWeighted(const param::MIRParametrisation &);

    virtual ~MethodWeighted();

    virtual void execute(data::MIRField &field, const atlas::grid::Grid &in, const atlas::grid::Grid &out, util::MIRStatistics& statistics) const;

    virtual void hash( eckit::MD5 & ) const;

  private:

    double lsmWeightAdjustement_;

    virtual const char *name() const = 0;

    virtual void assemble(WeightMatrix &W, const atlas::grid::Grid &in, const atlas::grid::Grid &out, util::MIRStatistics& statistics) const = 0;

    /// Update interpolation weigths matrix to account for missing values
    WeightMatrix applyMissingValues(const WeightMatrix &W, data::MIRField &field, size_t which, util::MIRStatistics& statistics) const;

    /// Update interpolation weigths matrix to account for field masked values
    virtual void applyMasks(WeightMatrix &W, const lsm::LandSeaMasks &) const;

    virtual const WeightMatrix &getMatrix(const atlas::grid::Grid &in, const atlas::grid::Grid &out, util::MIRStatistics& statistics) const;

    virtual lsm::LandSeaMasks getMasks(const atlas::grid::Grid &in, const atlas::grid::Grid &out, util::MIRStatistics& statistics) const;

    void computeMatrixWeights(const atlas::grid::Grid &in, const atlas::grid::Grid &out, WeightMatrix &W, util::MIRStatistics& statistics) const;

};

//----------------------------------------------------------------------------------------------------------------------

}  // namespace method
}  // namespace mir

#endif

