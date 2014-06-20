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
/// @date June 2014

#ifndef mir_Masks_H
#define mir_Masks_H

#include <string>

#include "eckit/memory/NonCopyable.h"

#include "atlas/grid/Grid.h"
#include "atlas/grid/FieldSet.h"

#include "mir/Weights.h"

//------------------------------------------------------------------------------------------------------

namespace mir {

//------------------------------------------------------------------------------------------------------

class Masks : private eckit::NonCopyable {

public: // types

    typedef atlas::grid::Grid        Grid;
    typedef atlas::grid::Grid::Point Point;
    typedef atlas::grid::FieldHandle FieldHandle;
    typedef atlas::grid::FieldSet    FieldSet;

    typedef Weights::Matrix Matrix;

public: // methods

    Masks();

    virtual ~Masks();

//    virtual std::string classname() const = 0;

    void assemble( const FieldHandle& mask, const Grid& inp, const Grid& out, Weights::Matrix& W ) const;

protected: // methods

//    virtual void compute( const FieldHandle& mask,
//                          const Grid& inp,
//                          const Grid& out,
//                          Weights::Matrix& W ) const = 0;

};

//------------------------------------------------------------------------------------------------------

} // namespace mir

#endif
