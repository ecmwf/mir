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

#include <list>
#include <utility>
#include "atlas/array/ArrayView.h"
#include "atlas/field/Field.h"
#include "atlas/grid/Grid.h"
#include "atlas/internals/Parameters.h"  // for LON, LAT
#include "atlas/mesh/Connectivity.h"
#include "atlas/mesh/Elements.h"
#include "atlas/mesh/Mesh.h"
#include "atlas/mesh/Nodes.h"
#include "eckit/geometry/Point3.h"


namespace mir {
namespace method {


namespace {


using atlas::idx_t;
using atlas::internals::LON;
using atlas::internals::LAT;
typedef std::pair< idx_t, idx_t > edge_t;
typedef std::list< edge_t > edge_list_t;


static edge_list_t getParallelEdges(
        const atlas::grid::Domain& parallel,
        const atlas::mesh::Connectivity& cells_nodes_connectivity,
        const atlas::array::ArrayView<double, 2>& coords ) {
    edge_list_t edges;

    std::vector<bool> onParallel(coords.shape(0));
    for (size_t ip = 0; ip < coords.shape(0); ++ip) {
        onParallel[ip] = parallel.contains(coords[ip][LON], coords[ip][LAT]);
    }

    for (size_t elem_id = 0; elem_id < cells_nodes_connectivity.rows(); ++elem_id) {
        const size_t nb_cols = cells_nodes_connectivity.cols(elem_id);
        ASSERT(nb_cols == 3 || nb_cols == 4);

        for (size_t i = 0; i < nb_cols; ++i) {
            const idx_t p1 = cells_nodes_connectivity(elem_id,   i );
            const idx_t p2 = cells_nodes_connectivity(elem_id, (i+1) % nb_cols);
            if (onParallel[p1] && onParallel[p2]) {
                edges.push_front(edge_t(p1, p2));
            }
        }
    }

    return edges;
}


}  // (anonymous namespace)


void AddParallelEdgesConnectivity::operator()(const atlas::grid::Domain& domain, atlas::mesh::Mesh& mesh) const {

    // build list of North and South parallels edges
    edge_list_t edges;
    bool north = false;
    if (!domain.includesPoleNorth() && domain.north() < 0.) {
        north = true;
        edges = getParallelEdges(
                    atlas::grid::Domain(domain.north(), 0, domain.north(), 360),
                    mesh.cells().node_connectivity(),
                    atlas::array::ArrayView<double, 2>(mesh.nodes().lonlat()) );
    } else if (!domain.includesPoleSouth() && domain.south() > 0.) {
        edges = getParallelEdges(
                    atlas::grid::Domain(domain.south(), 0, domain.south(), 360),
                    mesh.cells().node_connectivity(),
                    atlas::array::ArrayView<double, 2>(mesh.nodes().lonlat()) );
    }

    if (edges.empty()) {
        // nothing to do
        return;
    }


    // resize nodes
    atlas::mesh::Nodes& nodes = mesh.nodes();
    const size_t nbRealPts = nodes.size();
    nodes.metadata().set<size_t>("NbRealPts", nbRealPts);
    nodes.resize(nbRealPts + 1);


    // resize connectivity
    atlas::mesh::Connectivity& connect = mesh.cells().node_connectivity();
    size_t lastElement = connect.rows();
    idx_t triangle[3];
    connect.add(edges.size(), 3);


    atlas::array::ArrayView<double, 2> coords(nodes.field("xyz"));
    atlas::array::ArrayView<double, 2> lonlat(nodes.lonlat());
    atlas::array::ArrayView<atlas::gidx_t, 1> gidx(nodes.global_index());


    // add parallel elements touching pole
    const size_t i = nbRealPts;

    lonlat(i, LON) = 0;
    lonlat(i, LAT) = north? 90 : -90;
    eckit::geometry::lonlat_to_3d(lonlat[i].data(), coords[i].data());
    gidx(i) = idx_t(i + 1);

    triangle[0] = i;
    for (const edge_t& edge: edges) {
        triangle[1] = edge.second;  // order is {pole, E2, E1}
        triangle[2] = edge.first;   // ...
        connect.set(lastElement++, triangle);
    }
}


}  // namespace method
}  // namespace mir
