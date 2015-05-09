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


#ifndef mir_method_FiniteElement_H
#define mir_method_FiniteElement_H

#include "eckit/memory/ScopedPtr.h"

#include "eckit/maths/Eigen.h"
#include "eckit/geometry/Point3.h"

#include "atlas/Mesh.h"
#include "atlas/Field.h"
#include "atlas/PointIndex3.h"

#include "mir/method/MethodWeighted.h"


namespace mir {
namespace method {


class FiniteElement: public MethodWeighted {

  mutable eckit::ScopedPtr<atlas::PointIndex3> ptree;

  mutable size_t ip_;

  mutable size_t nb_triags;
  mutable size_t inp_npts;

  mutable atlas::FieldT<double>* picoords;
  mutable atlas::FieldT<int>* ptriag_nodes;

public:

    typedef eckit::geometry::Point3 Point;

    FiniteElement(const param::MIRParametrisation&);

    virtual ~FiniteElement();

protected:

    virtual void hash( eckit::MD5& ) const;

private:

// -- Methods
    bool project_point_to_triangle( Point &p, Eigen::Vector3d& phi, int idx[3], const size_t k ) const;

// -- Overridden methods

    virtual void assemble(WeightMatrix& W, const atlas::Grid& in, const atlas::Grid& out) const;
    virtual void print(std::ostream&) const;
    virtual const char* name() const;

    friend std::ostream& operator<<(std::ostream& s,const FiniteElement& p) {
        p.print(s);
        return s;
    }

};

}  // namespace method
}  // namespace mir

#endif

