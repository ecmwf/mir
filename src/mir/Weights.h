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

#include "eckit/maths/Eigen.h"

#include "eckit/memory/NonCopyable.h"

#include "atlas/Grid.h"

//------------------------------------------------------------------------------------------------------

namespace mir {

//------------------------------------------------------------------------------------------------------

class Weights : private eckit::NonCopyable {

public: // types

    typedef Eigen::SparseMatrix<double,Eigen::RowMajor> Matrix;

public: // methods

    Weights();

    virtual ~Weights();

    virtual std::string classname() const = 0;

	void assemble( const atlas::Grid& in, const atlas::Grid& out, Weights::Matrix& W ) const;

protected: // methods

	virtual void compute( atlas::Grid& i_mesh, atlas::Grid& o_mesh, Weights::Matrix& W ) const = 0;

	std::string hash( const atlas::Grid& in, const atlas::Grid& out ) const;

};

//------------------------------------------------------------------------------------------------------

} // namespace mir

#endif
