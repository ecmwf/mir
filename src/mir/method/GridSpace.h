/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Tiago Quintino
/// @date   May 2016

#ifndef mir_method_GridSpace_H
#define mir_method_GridSpace_H

#include <vector>

#include "eckit/memory/NonCopyable.h"
#include "eckit/memory/ScopedPtr.h"

namespace atlas {
    namespace grid { class Grid; }
    namespace mesh { class Mesh; }

    namespace array {
    class Array;
    template <typename DATA_TYPE, int RANK > class ArrayView;
    }
}

namespace mir {
namespace method {

//----------------------------------------------------------------------------------------------------------------------

class GridSpace : private eckit::NonCopyable {

public: // methods

    GridSpace(const atlas::grid::Grid& grid);

    ~GridSpace();

    const atlas::grid::Grid& grid() const { return grid_; }

    atlas::mesh::Mesh& mesh() const;

    const std::vector<double>& coords() const;

    atlas::array::ArrayView<double,2> coordsView() const;

private: // members

    const atlas::grid::Grid& grid_;

    mutable eckit::ScopedPtr<atlas::mesh::Mesh> mesh_;

    mutable std::vector<double> coords_;

};

//----------------------------------------------------------------------------------------------------------------------

}  // namespace method
}  // namespace mir

#endif

