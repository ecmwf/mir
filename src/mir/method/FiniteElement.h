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
/// @date May 2015


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

public:

    typedef eckit::geometry::Point3 Point;

public:

    FiniteElement(const param::MIRParametrisation&);

    virtual ~FiniteElement();

  protected:

    virtual void hash( eckit::MD5& ) const;

  private:

    struct MeshStats {

        size_t nb_triags;
        size_t nb_quads;
        size_t inp_npts;
        size_t out_npts;

        size_t nbElems() const { return nb_triags + nb_quads; }

        void print(std::ostream& s) const {
            s << "MeshStats[nb_triags=" << nb_triags
              << ",nb_quads=" << nb_quads
              << ",inp_npts=" << inp_npts
              << ",out_npts=" << out_npts << "]";
        }

        friend std::ostream& operator<<(std::ostream& s, const MeshStats& p) {
          p.print(s);
          return s;
        }
    };

    struct Phi
    {
        std::vector<int>    idx;
        std::vector<double> w;

        Phi() { idx.reserve(4); w.reserve(4); }

        size_t size() const { return idx.size(); }
        void resize(size_t s) { idx.resize(s); w.resize(s); }

        void print(std::ostream& s) const;

        friend std::ostream& operator<<(std::ostream& s, const Phi& p) {
          p.print(s);
          return s;
        }

    };

    mutable atlas::FieldT<double>* picoords;
    mutable atlas::FieldT<int>* ptriag_nodes;
    mutable atlas::FieldT<int>* pquads_nodes;

    mutable std::vector<Point> failed_;
    mutable Phi phi_;

// -- Methods

    bool projectPointToElements(const MeshStats& stats,
                                Point& p,
                                atlas::ElemIndex3::NodeList::const_iterator start,
                                atlas::ElemIndex3::NodeList::const_iterator finish) const;

// -- Overridden methods

    virtual void assemble(WeightMatrix& W, const atlas::Grid& in, const atlas::Grid& out) const;
    virtual void print(std::ostream&) const;
    virtual const char* name() const;

};


}  // namespace method
}  // namespace mir

#endif

