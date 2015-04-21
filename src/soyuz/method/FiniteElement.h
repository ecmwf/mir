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
/// @date Mar 2014

#ifndef soyuz_method_FiniteElement_H
#define soyuz_method_FiniteElement_H

#include "eckit/memory/ScopedPtr.h"

#include "eckit/maths/Eigen.h"

#include "atlas/Mesh.h"
#include "atlas/Field.h"

#include "soyuz/method/MethodWeighted.h"


namespace mir {
namespace method {


class FiniteElement: public MethodWeighted {
  public:

// -- Exceptions
    // None

// -- Contructors

    FiniteElement(const MIRParametrisation& param);

// -- Destructor

    virtual ~FiniteElement();

// -- Convertors
    // None

// -- Operators
    // None

// -- Methods
    // None

// -- Overridden methods
    virtual void assemble(MethodWeighted::Matrix& W) const;

// -- Class members
    // None

// -- Class methods
    // None

  protected:

// -- Members


// -- Methods

    virtual void print(std::ostream&) const;

// -- Overridden methods
    // None

// -- Class members
    // None

// -- Class methods
    // None

  private:

// -- Members
    mutable eckit::ScopedPtr<atlas::PointIndex3> ptree;

    mutable size_t ip_;

    mutable size_t nb_triags;
    mutable size_t inp_npts;

    mutable atlas::FieldT<double>* picoords;
    mutable atlas::FieldT<int>* ptriag_nodes;

// -- Methods
    bool project_point_to_triangle( Point &p, Eigen::Vector3d& phi, int idx[3], const size_t k ) const;

// -- Overridden methods
    // None

// -- Class members
    // None

// -- Class methods
    // None

// -- Friends

    friend std::ostream& operator<<(std::ostream& s,const FiniteElement& p) {
        p.print(s);
        return s;
    }

};


}  // namespace method
}  // namespace mir
#endif

