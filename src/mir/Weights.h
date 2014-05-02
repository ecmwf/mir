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

//------------------------------------------------------------------------------------------------------

namespace mir {

//------------------------------------------------------------------------------------------------------

class Weights : private eckit::NonCopyable {
public:

    typedef atlas::grid::Grid        Grid;
    typedef atlas::grid::Grid::Point Point;

    Weights();

    virtual ~Weights();

    virtual std::string classname() const = 0;

    void assemble( Grid& in, Grid& out, Eigen::SparseMatrix<double>& W ) const;

protected:

    virtual void compute( Grid& i_mesh, Grid& o_mesh, Eigen::SparseMatrix<double>& W ) const = 0;

    std::string hash( const Grid& in, const Grid& out ) const;

};

//------------------------------------------------------------------------------------------------------

} // namespace mir

#endif
