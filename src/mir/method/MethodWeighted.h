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

#ifndef soyuz_method_MethodWeighted_H
#define soyuz_method_MethodWeighted_H


#include <string>

#include "eckit/maths/Eigen.h"
#include "atlas/PointIndex3.h"

#include "mir/method/Method.h"


namespace atlas {
class Grid;
}


namespace mir {

namespace lsm {
class LandSeaMask;
}

namespace method {


class MethodWeighted : public Method {
  public:

// -- Definitions

    typedef Eigen::SparseMatrix< double, Eigen::RowMajor > Matrix;
    typedef eckit::geometry::Point3 Point;

// -- Exceptions
    // None

// -- Contructors
    MethodWeighted(const param::MIRParametrisation&);

// -- Destructor
    virtual ~MethodWeighted();

// -- Convertors
    // None

// -- Operators
    // None

// -- Methods
    // None

// -- Overridden methods
    virtual void execute(data::MIRField& field, const atlas::Grid& in, const atlas::Grid& out) const;

// -- Class members
    // None

// -- Class methods
    // None

  protected:

// -- Members
    // None

// -- Methods

    virtual const char* name() const = 0;
    virtual void assemble(Matrix& W, const atlas::Grid& in, const atlas::Grid& out) const = 0;

    /// Update interpolation weigths matrix to account for missing values
    Matrix applyMissingValues(const MethodWeighted::Matrix& W, data::MIRField& field, size_t which) const;

    /// Update interpolation weigths matrix to account for field masked values
    void applyInputMask(Matrix& W, const atlas::Grid& in, const atlas::Grid& out, const lsm::LandSeaMask&) const;
    void applyOutputMask(Matrix& W, const atlas::Grid& in, const atlas::Grid& out, const lsm::LandSeaMask&) const;
    void applyBothMask(Matrix& W, const atlas::Grid& in, const atlas::Grid& out, const lsm::LandSeaMask&, const lsm::LandSeaMask&) const;

    std::string hash(const atlas::Grid& in, const atlas::Grid& out) const;

    const Matrix& getMatrix(const atlas::Grid& in, const atlas::Grid& out) const;


// -- Overridden methods
    // None

// -- Class members

// -- Class methods
    // None

  private:

// -- Methods
    // None

// -- Overridden methods
    // None

// -- Class members
    // None

// -- Class methods
    // None

// -- Friends

    friend std::ostream& operator<<(std::ostream& s, const MethodWeighted& p) {
        p.print(s);
        return s;
    }

};


}  // namespace method
}  // namespace mir
#endif

