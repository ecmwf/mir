/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Tiago Quintino
/// @author Pedro Maciel
/// @date Apr 2015

#ifndef soyuz_method_Bilinear_H
#define soyuz_method_Bilinear_H

#include "mir/method/MethodWeighted.h"


namespace eckit {
namespace geometry {
class Point3;
}
}


namespace mir {
namespace method {


class Bilinear: public MethodWeighted {
  public:

// -- Exceptions
    // None

// -- Contructors
    Bilinear(const param::MIRParametrisation&);

// -- Destructor
    virtual ~Bilinear();

// -- Convertors
    // None

// -- Operators
    // None

// -- Methods
    // None

// -- Overridden methods

// -- Class members
    // None

// -- Class methods
    // None

  protected:

// -- Members


// -- Methods


// -- Overridden methods
    // None

// -- Class members
    // None

// -- Class methods
    // None

  private:

// -- Methods
    double crossProduct(const eckit::geometry::Point3& a, const eckit::geometry::Point3& b) const;
    bool formClockwiseTriangle(const eckit::geometry::Point3& a, const eckit::geometry::Point3& b, const eckit::geometry::Point3& c) const;
    void sort4Clockwise(std::vector<eckit::geometry::Point3>& points) const ;

// -- Overridden methods

    virtual void assemble(MethodWeighted::Matrix& W, const atlas::Grid& in, const atlas::Grid& out) const;
    virtual void print(std::ostream&) const;
    virtual const char* name() const;

// -- Class members
    // None

// -- Class methods
    // None

// -- Friends

    friend std::ostream& operator<<(std::ostream& s,const Bilinear& p) {
        p.print(s);
        return s;
    }

};


}  // namespace method
}  // namespace mir
#endif

