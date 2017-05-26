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
#include "atlas/array.h"
#include "atlas/grid/Grid.h"
#include "atlas/mesh/Mesh.h"
#include "mir/method/MethodWeighted.h"
#include "atlas/domain/detail/RectangularDomain.h"


namespace mir {
namespace method {


GridSpace::GridSpace(const atlas::Grid& grid, const MethodWeighted& method) :
    method_(method),
    grid_(grid),
    mesh_(0),
    coordsLonLat_(0) {

    // FIXME this is the ugliest code ever, it needs to be redesigned
    try {
        const atlas::Domain::Implementation* impl = grid_.domain().get();
        ASSERT(impl);

        const atlas::domain::RectangularDomain& rectangle = dynamic_cast< const atlas::domain::RectangularDomain& >(*impl);
        double N = rectangle.ymax();
        double W = rectangle.xmin();
        double S = rectangle.ymin();
        double E = rectangle.xmax();
        domain_ = util::Domain(N, W, S, E);

    } catch (const std::bad_cast&) {
        throw eckit::SeriousBug("Could not get a RectangularDomain from an Atlas grid", Here());
    }
}


const util::Domain& GridSpace::domain() const {
    return domain_;
}


const atlas::Grid& GridSpace::grid() const {
    return grid_;
}


atlas::Mesh& GridSpace::mesh() const {
    if (mesh_ == 0) {
        mesh_ = &method_.generateMeshAndCache(grid_);
    }
    return *mesh_;
}


const atlas::array::Array& GridSpace::coordsLonLat() const {
    using namespace atlas::array;

    if (!coordsLonLat_) {
        coordsLonLat_.reset(Array::create< double >(grid_.size(), 2));
        ArrayView< double, 2 > lonlat = make_view< double, 2 >(*coordsLonLat_);

        size_t i = 0;
        for (atlas::PointLonLat p : grid_.lonlat()) {
            lonlat(i, 0) = p.lon();
            lonlat(i, 1) = p.lat();
            ++i;
        }
    }

    return *coordsLonLat_;
}


const atlas::array::Array& GridSpace::coordsXYZ() const {
    using namespace atlas::array;

    if (!coordsXYZ_) {
        coordsXYZ_.reset(Array::create< double >(grid_.size(), 3));
        ArrayView< double, 2 > xyz = make_view< double, 2 >(*coordsXYZ_);

        size_t i = 0;
        for (atlas::PointLonLat p : grid_.lonlat()) {
            atlas::PointXYZ x = atlas::lonlat_to_geocentric(p);
            xyz(i, 0) = x.x();
            xyz(i, 1) = x.y();
            xyz(i, 2) = x.z();
            ++i;
        }
    }

    return *coordsXYZ_;
}


}  // namespace method
}  // namespace mir
