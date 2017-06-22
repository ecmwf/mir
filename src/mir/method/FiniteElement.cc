/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Tiago Quintino
/// @author Pedro Maciel
/// @date May 2015


#include "mir/method/FiniteElement.h"

#include <algorithm>
#include <limits>
#include <forward_list>
#include "eckit/config/Resource.h"
#include "eckit/log/BigNum.h"
#include "eckit/log/ETA.h"
#include "eckit/log/Plural.h"
#include "eckit/log/ResourceUsage.h"
#include "eckit/log/Seconds.h"
#include "eckit/log/Timer.h"
#include "eckit/memory/ScopedPtr.h"
#include "eckit/utils/MD5.h"
#include "atlas/interpolation/element/Quad3D.h"
#include "atlas/interpolation/element/Triag3D.h"
#include "atlas/interpolation/method/PointIndex3.h"
#include "atlas/interpolation/method/Ray.h"
#include "atlas/mesh/ElementType.h"
#include "atlas/mesh/Elements.h"
#include "atlas/mesh/Nodes.h"
#include "atlas/mesh/actions/BuildCellCentres.h"
#include "atlas/mesh/actions/BuildXYZField.h"
#include "atlas/output/Gmsh.h"
#include "mir/config/LibMir.h"
#include "mir/method/AddParallelEdgesConnectivity.h"
#include "mir/util/MIRGrid.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Representation.h"


namespace mir {
namespace method {


namespace {


// try to project to 20% of total number elements before giving up
static const double maxFractionElemsToTry = 0.2;

// epsilon used to scale edge tolerance when projecting ray to intesect element
static const double parametricEpsilon = 1e-16;


using eckit::geometry::LON;
using eckit::geometry::LAT;
typedef std::vector< WeightMatrix::Triplet > triplet_vector_t;
typedef atlas::interpolation::method::ElemIndex3 element_tree_t;


struct MeshStats {

    size_t inp_ncells;
    size_t inp_npts;
    size_t out_npts;

    MeshStats(): inp_ncells(0), inp_npts(0), out_npts(0) {}

    void print(std::ostream &s) const {
        s << "MeshStats["
              "nb_cells=" << eckit::BigNum(inp_ncells)
          << ",inp_npts=" << eckit::BigNum(inp_npts)
          << ",out_npts=" << eckit::BigNum(out_npts)
          << "]";
    }

    friend std::ostream &operator<<(std::ostream &s, const MeshStats &p) {
        p.print(s);
        return s;
    }
};


static void normalise(triplet_vector_t& triplets)
{
    // sum all calculated weights for normalisation
    double sum = 0.0;
    for (size_t j = 0; j < triplets.size(); ++j) {
        sum += triplets[j].value();
    }

    // now normalise all weights according to the total
    const double invSum = 1.0 / sum;
    for (size_t j = 0; j < triplets.size(); ++j) {
        triplets[j].value() *= invSum;
    }
}


/// Find in which element the point is contained by projecting the point with each nearest element
static triplet_vector_t projectPointTo3DElements(
        size_t nbInputPoints,
        const atlas::array::ArrayView<double, 2> &icoords,
        const atlas::mesh::HybridElements::Connectivity& connectivity,
        const FiniteElement::Point &p,
        size_t ip,
        size_t firstVirtualPoint,
        element_tree_t::NodeList::const_iterator start,
        element_tree_t::NodeList::const_iterator finish ) {

    ASSERT(start != finish);

    triplet_vector_t triplets;

    bool mustNormalise = false;
    size_t idx[4];
    double w[4];
    atlas::interpolation::method::Ray ray( p.data() );

    for (element_tree_t::NodeList::const_iterator itc = start; itc != finish; ++itc) {

        const size_t elem_id = (*itc).value().payload();
        ASSERT(elem_id < connectivity.rows());

        /* assumes:
         * - nb_cols == 3 implies triangle
         * - nb_cols == 4 implies quadrilateral
         * - no other element is supported at the time
         */
        const size_t nb_cols = connectivity.cols(elem_id);
        ASSERT(nb_cols == 3 || nb_cols == 4);

        for (size_t i = 0; i < nb_cols; ++i) {
            idx[i] = size_t(connectivity(elem_id, i));
            ASSERT(idx[i] < nbInputPoints);
        }

        if (nb_cols == 3) {

            /* triangle */
            atlas::interpolation::element::Triag3D triag(
                    icoords[idx[0]].data(),
                    icoords[idx[1]].data(),
                    icoords[idx[2]].data());

            // pick an epsilon based on a characteristic length (sqrt(area))
            // (this scales linearly so it better compares with linear weights u,v,w)
            const double edgeEpsilon = parametricEpsilon * std::sqrt(triag.area());
            ASSERT(edgeEpsilon >= 0);

            atlas::interpolation::method::Intersect is = triag.intersects(ray, edgeEpsilon);

            if (is) {

                // weights are the linear Lagrange function evaluated at u,v (aka barycentric coordinates)
                w[0] = 1. - is.u - is.v;
                w[1] = is.u;
                w[2] = is.v;

                for (size_t i = 0; i < 3; ++i)
                {
                    if(idx[i] < firstVirtualPoint)
                        triplets.push_back( WeightMatrix::Triplet( ip, idx[i], w[i] ) );
                    else
                        mustNormalise = true;
                }

                break; // stop looking for elements
            }

        } else {

            /* quadrilateral */
            atlas::interpolation::element::Quad3D quad(
                    icoords[idx[0]].data(),
                    icoords[idx[1]].data(),
                    icoords[idx[2]].data(),
                    icoords[idx[3]].data() );

            if ( !quad.validate() ) { // somewhat expensive sanity check
                eckit::Log::warning() << "Invalid Quad : " << quad << std::endl;
                throw eckit::SeriousBug("Found invalid quadrilateral in mesh", Here());
            }

            // pick an epsilon based on a characteristic length (sqrt(area))
            // (this scales linearly so it better compares with linear weights u,v,w)
            const double edgeEpsilon = parametricEpsilon * std::sqrt(quad.area());
            ASSERT(edgeEpsilon >= 0);

            atlas::interpolation::method::Intersect is = quad.intersects(ray, edgeEpsilon);

            if (is) {

                // weights are the bilinear Lagrange function evaluated at u,v
                w[0] = (1. - is.u) * (1. - is.v);
                w[1] =       is.u  * (1. - is.v);
                w[2] =       is.u  *       is.v ;
                w[3] = (1. - is.u) *       is.v ;


                for (size_t i = 0; i < 4; ++i) {
                    if(idx[i] < firstVirtualPoint)
                        triplets.push_back( WeightMatrix::Triplet( ip, idx[i], w[i] ) );
                    else
                        mustNormalise = true;
                }

                break; // stop looking for elements
            }
        }

    } // loop over nearest elements

    // at least one of the nodes of element shouldn't be virtual
    if (!triplets.empty() && mustNormalise) {
        normalise(triplets);
    }

    return triplets;
}


}  // (anonymous namespace)


FiniteElement::FiniteElement(const param::MIRParametrisation &param) :
    MethodWeighted(param) {

    meshgenparams_.meshGenerator_ = "structured";
    parametrisation_.get("meshgenerator", meshgenparams_.meshGenerator_);
}


FiniteElement::~FiniteElement() {
}


void FiniteElement::hash(eckit::MD5&) const {
}


void FiniteElement::assemble(WeightMatrix& W, const repres::Representation& rin, const repres::Representation& rout) const {

    util::MIRGrid in(rin.grid(meshgenparams_));
    util::MIRGrid out(rout.grid(meshgenparams_));

    eckit::Log::debug<LibMir>() << "FiniteElement::assemble (input: " << rin << ", output: " << rout << ")" << std::endl;


    // write input/output meshes
    static bool dumpMesh = eckit::Resource<bool>("$MIR_DUMP_MESH", false);
    if (dumpMesh) {
        eckit::Log::debug<LibMir>() << "Dumping input mesh to 'input.msh'" << std::endl;
        atlas::output::Gmsh("input.msh", atlas::util::Config("coordinates", "xyz")).write(in.mesh());

        eckit::Log::debug<LibMir>() << "Dumping output mesh to 'output.msh'" << std::endl;
        atlas::output::Gmsh("output.msh", atlas::util::Config("coordinates", "xyz")).write(out.mesh());
    }


    // if domain does not include poles, we might need to recover the parallel edges
    {
        eckit::TraceTimer<LibMir> timer("AddParallelEdgesConnectivity");
        AddParallelEdgesConnectivity()(in.domain(), in.mesh());
    }


    // generate barycenters of each triangle & insert them on a kd-tree
    {
        eckit::ResourceUsage usage("create_cell_centres");
        eckit::TraceTimer<LibMir> timer("Tesselation::create_cell_centres");
        atlas::mesh::actions::BuildCellCentres()(in.mesh());
    }

    eckit::ScopedPtr<element_tree_t> eTree;
    {
        eckit::ResourceUsage usage("create_element_centre_index");
        eckit::TraceTimer<LibMir> timer("create_element_centre_index");
        eTree.reset( atlas::interpolation::method::create_element_centre_index(in.mesh()) );
    }


    // input mesh
    const util::Domain& inDomain = in.domain();
    const atlas::mesh::Nodes& i_nodes = in.mesh().nodes();
    atlas::array::ArrayView<double, 2> icoords = atlas::array::make_view< double, 2 >( i_nodes.field( "xyz" ));

    size_t firstVirtualPoint = std::numeric_limits<size_t>::max();
    if (i_nodes.metadata().has("NbRealPts")) {
        firstVirtualPoint = i_nodes.metadata().get<size_t>("NbRealPts");
    }


    // output mesh
    atlas::array::ArrayView<double, 2> ocoords = atlas::array::make_view< double, 2 >(out.coordsXYZ());
    atlas::array::ArrayView<double, 2> olonlat = atlas::array::make_view< double, 2 >(out.coordsLonLat());


    MeshStats stats;
    stats.inp_ncells = in.mesh().cells().size();
    stats.inp_npts   = i_nodes.size();
    stats.out_npts   = out.size();
    eckit::Log::debug<LibMir>() << stats << std::endl;


    // weights -- one per vertex of element, triangles (3) or quads (4)
    triplet_vector_t weights_triplets; // structure to fill-in sparse matrix
    weights_triplets.reserve( stats.out_npts * 4 );        // preallocate space as if all elements where quads


    // search nearest k cell centres
    const size_t nbElementsMaximum = std::max<size_t>(64, stats.inp_ncells * maxFractionElemsToTry);
    size_t nbElementsSearched = 0;
    size_t nbProjections = 0;
    size_t nbFailures = 0;
    std::forward_list<size_t> failures;

    {
        eckit::Log::debug<LibMir>() << "Projecting " << eckit::Plural(stats.out_npts, "output point") << " to input mesh " << rin << std::endl;
        eckit::TraceTimer<LibMir> timerProj("Projecting");

        const atlas::mesh::HybridElements::Connectivity& connectivity = in.mesh(*this).cells().node_connectivity();
        for ( size_t ip = 0; ip < stats.out_npts; ++ip ) {

            if (ip && (ip % 10000 == 0)) {
                double rate = ip / timerProj.elapsed();
                eckit::Log::debug<LibMir>()
                        << eckit::BigNum(ip) << " ..."  << eckit::Seconds(timerProj.elapsed())
                        << ", rate: " << rate << " points/s, ETA: "
                        << eckit::ETA( (stats.out_npts - ip) / rate )
                        << std::endl;
            }

            if (inDomain.contains(olonlat(ip, LAT), olonlat(ip, LON))) {
                bool success = false;

                // 3D point to lookup
                Point p(ocoords[ip].data());

                // 3D projection
                for (size_t k = 0, kPrevious = 0; !success && k < nbElementsMaximum; kPrevious = k) {
                    k = std::min(k? 2 * k : 1, nbElementsMaximum);
                    nbElementsSearched = std::max(k, nbElementsSearched);

                    // loop over closest elements (enlarging range if failing projection)
                    element_tree_t::NodeList cs = eTree->kNearestNeighbours(p, k);

                    triplet_vector_t triplets = projectPointTo3DElements(
                                stats.inp_npts,
                                icoords,
                                connectivity,
                                p,
                                ip,
                                firstVirtualPoint,
                                cs.begin() + element_tree_t::NodeList::difference_type(kPrevious),
                                cs.end() );

                    if (triplets.size()) {
                        std::copy(triplets.begin(), triplets.end(), std::back_inserter(weights_triplets));
                        success = true;
                        ++nbProjections;
                    }
                }

                if (!success) {
                    // If this fails, consider lowering atlas::grid::parametricEpsilon
                    failures.push_front(ip);
                    ++nbFailures;
                }
            }
        }
    }

    eckit::Log::debug<LibMir>()
            << "Projected " << eckit::BigNum(nbProjections)
            << " of " << eckit::Plural(stats.out_npts, "point")
            << " (" << eckit::Plural(nbFailures, "failure") << ")\n"
            << "Maximum neighbours searched was " << eckit::Plural(nbElementsSearched, "element")
            << std::endl;

    if (nbFailures) {
        std::stringstream msg;
        msg << "Failed to project " << eckit::Plural(nbFailures, "point");
        eckit::Log::debug<LibMir>() << msg.str() << ":";
        size_t count = 0;
        for (const size_t& ip: failures) {
            eckit::Log::debug<LibMir>() << "\n\tpoint " << ip << " (lon, lat) = (" << olonlat(ip, LON) << ", " << olonlat(ip, LAT) << ")";
            if (++count > 10) {
                eckit::Log::debug<LibMir>() << "\n\t...";
                break;
            }
        }
        eckit::Log::debug<LibMir>() << std::endl;
        throw eckit::SeriousBug(msg.str());
    }


    // fill sparse matrix
    W.setFromTriplets(weights_triplets);
}


}  // namespace method
}  // namespace mir

