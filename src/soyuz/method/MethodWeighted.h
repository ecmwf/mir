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
/// @date Oct 2013

#ifndef soyuz_method_MethodWeighted_H
#define soyuz_method_MethodWeighted_H


#include <string>

#include "eckit/maths/Eigen.h"
#include "atlas/PointIndex3.h"
#include "eckit/memory/NonCopyable.h"

#include "atlas/Grid.h"

#include "soyuz/method/Method.h"


namespace mir {
namespace method {


class MethodWeighted :
    public Method,
    private eckit::NonCopyable {
  public:

// -- Definitions

    typedef Eigen::SparseMatrix< double, Eigen::RowMajor > Matrix;
    typedef eckit::geometry::Point3 Point;

// -- Exceptions
    // None

// -- Contructors
    // None

// -- Destructor

    virtual ~MethodWeighted();

// -- Convertors
    // None

// -- Operators
    // None

// -- Methods
    // None

// -- Overridden methods
    virtual void execute(data::MIRField& field) const;

// -- Class members
    // None

// -- Class methods
    // None

  protected:

// -- Members
    // None

// -- Methods

    virtual void assemble(Matrix& W) const = 0;

    void applyMask(Matrix& W) const;

    std::string hash() const;

    virtual void print(std::ostream&) const {}

// -- Overridden methods
    // None

// -- Class members
    // None

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

    friend std::ostream& operator<<(std::ostream& s,const MethodWeighted& p) {
        p.print(s);
        return s;
    }

};


}  // namespace method
}  // namespace mir
#endif

