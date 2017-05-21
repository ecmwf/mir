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
/// @author Pedro Maciel
/// @date   May 2016


#include "mir/method/GridSpace.h"

#include "eckit/geometry/Point3.h"
#include "atlas/array/ArrayView.h"
#include "atlas/grid/Grid.h"
#include "atlas/mesh/Mesh.h"
#include "mir/method/MethodWeighted.h"


namespace mir {
namespace method {


using namespace atlas::array;


GridSpace::GridSpace(const atlas::grid::Grid& grid, const MethodWeighted& method) :
    method_(method),
    grid_(grid),
    mesh_(0),
    coordsLonLat_(0) {
}


const atlas::grid::Grid& GridSpace::grid() const {
    return grid_;
}


atlas::mesh::Mesh& GridSpace::mesh() const {
    if (mesh_ == 0) {
        mesh_ = &method_.generateMeshAndCache(grid_);
    }
    return *mesh_;
}


const std::vector<double>& GridSpace::coords() const {
    if (!coordsLonLat_.size()) {
        coordsLonLat_.resize(grid_.npts() * 2);
        grid_.fillLonLat(coordsLonLat_);
    }
    return coordsLonLat_;
}


ArrayView<double, 2> GridSpace::coordsLonLat() const {
    return ArrayView<double, 2>(&coords()[0], make_shape(grid_.npts(), 2));
}


ArrayView<double, 2> GridSpace::coordsXYZ() const {
    if (!coordsXYZ_.size()) {
        const size_t npts = grid_.npts();
        coordsXYZ_.resize(npts * 3);
        ArrayView<double, 2> lonlat = coordsLonLat();
        for (size_t n = 0; n < npts; ++n) {
            eckit::geometry::lonlat_to_3d(lonlat[n].data(), &coordsXYZ_[3 * n]);
        }
    }
    return ArrayView<double, 2>(&coordsXYZ_[0], make_shape(grid_.npts(), 3));
}


}  // namespace method
}  // namespace mir
