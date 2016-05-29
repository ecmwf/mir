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


#include "mir/method/GridSpace.h"

#include "atlas/grid/Grid.h"
#include "atlas/mesh/Mesh.h"
#include "atlas/array/ArrayView.h"


namespace mir {
namespace method {

//----------------------------------------------------------------------------------------------------------------------

GridSpace::GridSpace(const atlas::grid::Grid& grid) :
    grid_(grid),
    mesh_(0),
    coords_(0)
{

}

GridSpace::~GridSpace()
{
}

atlas::mesh::Mesh& GridSpace::mesh() const
{
    if(!mesh_) { mesh_.reset(new atlas::mesh::Mesh()); }
    return *mesh_;
}

const std::vector<double>& GridSpace::coords() const
{
    if(!coords_.size()) {
        coords_.resize(grid_.npts()*2);
        grid_.fillLonLat(coords_);
    }
    return coords_;
}

atlas::array::ArrayView<double,2> GridSpace::coordsView() const
{
    return atlas::array::ArrayView<double,2>( &coords()[0], atlas::array::make_shape(grid_.npts(), 2));
}

//----------------------------------------------------------------------------------------------------------------------

}  // namespace method
}  // namespace mir

