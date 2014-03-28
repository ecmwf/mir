/*
 * (C) Copyright 1996-2014 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Tiago Quintino
/// @date Mar 2014

#ifndef mir_FEInterpolator_H
#define mir_FEInterpolator_H

#include <memory>

#include "eckit/maths/Eigen.h"

#include "atlas/Mesh.hpp"
#include "atlas/Field.hpp"
#include "atlas/grid/PointIndex3.h"

//-----------------------------------------------------------------------------

namespace mir {

//-----------------------------------------------------------------------------

class FEInterpolator : private eckit::NonCopyable {

public: // methods

    void compute_weights( atlas::Mesh& i_mesh, atlas::Mesh& o_mesh, Eigen::SparseMatrix<double>& W );

private: // methods

    bool project_point_to_triangle( atlas::Point3 &p, Eigen::Vector3d& phi, int idx[3], const size_t k );

private: // members

    std::unique_ptr<atlas::PointIndex3> ptree;

    size_t ip_;

    size_t nb_triags;
    size_t inp_npts;

    atlas::FieldT<double>* picoords;
    atlas::FieldT<int>* ptriag_nodes;
};

//-----------------------------------------------------------------------------

} // namespace mir

#endif
