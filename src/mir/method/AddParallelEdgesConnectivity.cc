/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/method/AddParallelEdgesConnectivity.h"

#include "eckit/geometry/Point2.h"
#include "eckit/geometry/Point3.h"
#include "atlas/array/ArrayView.h"
#include "atlas/field/Field.h"
#include "atlas/grid/Grid.h"
#include "atlas/internals/Parameters.h"  // for LON, LAT
#include "atlas/mesh/Connectivity.h"
#include "atlas/mesh/Elements.h"
#include "atlas/mesh/Mesh.h"
#include "atlas/mesh/Nodes.h"
#include "mir/method/AddPoint.h"
#include "mir/method/GridSpace.h"
#include "mir/method/MethodWeighted.h"


namespace mir {
namespace method {


namespace {
static atlas::mesh::Connectivity* getParallelEdgesConnectivity(
        const atlas::grid::Domain& parallel,
        const size_t& poleIndex,
        const atlas::mesh::Connectivity& connectivity,
        const atlas::array::ArrayView<double, 2>& coords ) {
    using atlas::idx_t;
    using atlas::internals::LON;
    using atlas::internals::LAT;

    atlas::mesh::Connectivity* connect = new atlas::mesh::Connectivity();
    idx_t element[3];
    element[0] = idx_t(poleIndex);

    std::vector<bool> onParallel(coords.shape(0));
    for (size_t ip = 0; ip < coords.shape(0); ++ip) {
        onParallel[ip] = parallel.contains(coords[ip][LON], coords[ip][LAT]);
    }

    for (size_t elem_id = 0; elem_id < connectivity.rows(); ++elem_id) {
        const size_t nb_cols = connectivity.cols(elem_id);
        ASSERT(nb_cols == 3 || nb_cols == 4);

        for (size_t i = 0; i < nb_cols; ++i) {
            const idx_t p1 = connectivity(elem_id,   i );
            const idx_t p2 = connectivity(elem_id, (i+1) % nb_cols);
            if (onParallel[p1] && onParallel[p2]) {
                connect->add(1, 3);
                element[1] = p2;
                element[2] = p1;
                connect->set(connect->rows() - 1, element);
            }
        }
    }

    return connect;
}
}  // (anonymous namespace)


void AddParallelEdgesConnectivity::operator()(const atlas::grid::Domain& domain, atlas::mesh::Mesh& mesh) const {
    atlas::mesh::Nodes& nodes = mesh.nodes();

    if (!domain.includesPoleNorth()) {
        size_t poleIndex = nodes.size();
        AddPoint()(mesh, eckit::geometry::LLPoint2(0, 90));

        const atlas::grid::Domain parallel(domain.north(), 0, domain.north(), 360);
        const atlas::array::ArrayView< double, 2 > coords(nodes.lonlat());
        atlas::mesh::Connectivity* connect = getParallelEdgesConnectivity(parallel, poleIndex, mesh.cells().node_connectivity(), coords);
        connect->rename("parallel-edges-north");
        nodes.add(connect);
    }

    if (!domain.includesPoleSouth()) {
        size_t poleIndex = nodes.size();
        AddPoint()(mesh, eckit::geometry::LLPoint2(0, -90));

        const atlas::grid::Domain parallel(domain.south(), 0, domain.south(), 360);
        const atlas::array::ArrayView< double, 2 > coords(nodes.lonlat());
        atlas::mesh::Connectivity* connect = getParallelEdgesConnectivity(parallel, poleIndex, mesh.cells().node_connectivity(), coords);
        connect->rename("parallel-edges-south");
        nodes.add(connect);
    }
}


}  // namespace method
}  // namespace mir
