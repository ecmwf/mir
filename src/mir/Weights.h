/*
 * (C) Copyright 1996-2014 ECMWF.
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

#ifndef mir_Weights_H
#define mir_Weights_H

#include <string>
#include <Eigen/Sparse>

#include "eckit/memory/NonCopyable.h"

#include "atlas/grid/Grid.h"

//-----------------------------------------------------------------------------

namespace atlas {
namespace grid {
    class Point2;
}
}


namespace mir {

//-----------------------------------------------------------------------------

class WeightEngine : private eckit::NonCopyable {
public:

    typedef atlas::grid::Grid::Point Point;

    WeightEngine() {}

    virtual ~WeightEngine() {}

    virtual std::string classname() const = 0;

    virtual void compute( atlas::Mesh& i_mesh, atlas::Mesh& o_mesh, Eigen::SparseMatrix<double>& W ) const = 0;

    // @todo make the input and output const surely
    void weights( atlas::grid::Grid& in, atlas::grid::Grid& out, Eigen::SparseMatrix<double>& W ) const;

    std::string weights_hash( const atlas::grid::Grid& in, const atlas::grid::Grid& out ) const;

};


} // namespace mir

#endif
