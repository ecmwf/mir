/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/method/AddPoint.h"

#include "eckit/geometry/Point2.h"
#include "eckit/geometry/Point3.h"
#include "atlas/field/Field.h"
#include "atlas/internals/Parameters.h"
#include "atlas/mesh/Mesh.h"
#include "atlas/mesh/Nodes.h"


namespace mir {
namespace method {


void add_point(
        size_t index,
        const eckit::geometry::LLPoint2& point,
        atlas::array::ArrayView<double, 2>& coords,
        atlas::array::ArrayView<double, 2>& lonlat,
        atlas::array::ArrayView<atlas::gidx_t, 1>& gidx) {

}


void AddPoint::operator()(atlas::mesh::Mesh& mesh, const eckit::geometry::LLPoint2& point) const {

    atlas::mesh::Nodes& nodes = mesh.nodes();
    size_t oldSize = nodes.size();
    size_t index = oldSize;

    // resize the fields
    nodes.resize(oldSize + 1);

    atlas::array::ArrayView<double, 2>        coords (nodes.field("xyz"));
    atlas::array::ArrayView<double, 2>        lonlat (nodes.lonlat());
    atlas::array::ArrayView<atlas::gidx_t, 1> gidx   (nodes.global_index());

    lonlat(index, atlas::internals::LON) = point.lon();
    lonlat(index, atlas::internals::LAT) = point.lat();

    eckit::geometry::lonlat_to_3d(lonlat[index].data(), coords[index].data());
    gidx(index) = index + 1;
}


}  // namespace method
}  // namespace mir
