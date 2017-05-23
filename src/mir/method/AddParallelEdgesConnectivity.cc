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
#include "eckit/exception/Exceptions.h"
#include "eckit/geometry/Point3.h"
#include "atlas/array/ArrayView.h"
#include "atlas/field/Field.h"
#include "atlas/grid/Grid.h"
#include "atlas/internals/Parameters.h"  // for LON, LAT
#include "atlas/mesh/Connectivity.h"
#include "atlas/mesh/ElementType.h"
#include "atlas/mesh/Elements.h"
#include "atlas/mesh/Mesh.h"
#include "atlas/mesh/Nodes.h"


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


size_t getTriangleType(const atlas::mesh::Mesh& mesh) {
    for (size_t t = 0; t < mesh.cells().nb_types(); ++t) {
        if (mesh.cells().element_type(t).name() == "Triangle") {
            return t;
        }
    }
    throw eckit::SeriousBug("Could not find element type 'Triangle'");
}


}  // (anonymous namespace)


void AddParallelEdgesConnectivity::operator()(const atlas::grid::Domain& domain, atlas::mesh::Mesh& mesh) const {

    // build list of North and South parallels edges
    edge_list_t edges;
    bool addNorthPole = false;

    if (domain.north() < 0.) {
        addNorthPole = true;
        edges = getParallelEdges(
                    atlas::grid::Domain(domain.north(), 0, domain.north(), 360),
                    mesh.cells().node_connectivity(),
                    atlas::array::ArrayView<double, 2>(mesh.nodes().lonlat()) );
    } else if (domain.south() > 0.) {
        edges = getParallelEdges(
                    atlas::grid::Domain(domain.south(), 0, domain.south(), 360),
                    mesh.cells().node_connectivity(),
                    atlas::array::ArrayView<double, 2>(mesh.nodes().lonlat()) );
    }

    if (edges.empty()) {
        // nothing to do
        return;
    }


    // resize nodes: add North/South pole
    const size_t nbOriginalPoints = mesh.nodes().size();
    const size_t P = nbOriginalPoints;  // North/South pole index
    mesh.nodes().resize(nbOriginalPoints + 1);

    atlas::mesh::Nodes& nodes = mesh.nodes();
    nodes.metadata().set<size_t>("NbRealPts", nbOriginalPoints);

    atlas::array::ArrayView<double, 2> coords(nodes.field("xyz"));
    atlas::array::ArrayView<double, 2> lonlat(nodes.lonlat());
    atlas::array::ArrayView<atlas::gidx_t, 1> index_nodes(nodes.global_index());

    lonlat(P, LON) = 0;
    lonlat(P, LAT) = addNorthPole? 90 : -90;
    eckit::geometry::lonlat_to_3d(lonlat[P].data(), coords[P].data());
    index_nodes(P) = idx_t(P + 1);


    // resize connectivity: add number-of-edges "parallel" elements touching pole
    atlas::mesh::Elements& elems = mesh.cells().elements(getTriangleType(mesh));
    const size_t nbOriginalTriags = elems.size();
    elems.add(edges.size());

    atlas::mesh::BlockConnectivity& connect = elems.node_connectivity();
    atlas::array::ArrayView<atlas::gidx_t, 1> index_elems(elems.global_index());

    const size_t offset = elems.begin();
    size_t j = nbOriginalTriags;
    idx_t triangle[3];
    triangle[0] = P;
    for (const edge_t& edge: edges) {
        triangle[1] = edge.second;  // order is {pole, E2, E1}
        triangle[2] = edge.first;   // ...
        index_elems(offset + j) = idx_t(j + 1);
        connect.set(j++, triangle);
    }
}


}  // namespace method
}  // namespace mir
