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
/// @date Apr 2015

#ifndef mir_method_MethodWeighted_H
#define mir_method_MethodWeighted_H

#include <string>

#include "mir/method/Method.h"
#include "mir/method/WeightCache.h"
#include "mir/method/WeightMatrix.h"

namespace atlas { class Grid; }

namespace mir {
namespace method {


class MethodWeighted : public Method {

    mutable WeightCache cache_;

  public:


    explicit MethodWeighted(const param::MIRParametrisation&);

    virtual ~MethodWeighted();

    virtual void execute(data::MIRField& field, const atlas::Grid& in, const atlas::Grid& out) const;

  protected:

    virtual const char* name() const = 0;

    virtual void assemble(WeightMatrix& W, const atlas::Grid& in, const atlas::Grid& out) const = 0;

    /// Update interpolation weigths matrix to account for missing values
    void applyMissingValues(WeightMatrix& W) const;

    /// Update interpolation weigths matrix to account for field masked values
    void applyMask(WeightMatrix& W) const;

    std::string hash(const atlas::Grid& in, const atlas::Grid& out) const;

  private:

    void compute_weights(const atlas::Grid& in, const atlas::Grid& out, WeightMatrix& W) const;

    friend std::ostream& operator<<(std::ostream& s,const MethodWeighted& p) {
        p.print(s);
        return s;
    }

};


}  // namespace method
}  // namespace mir
#endif

