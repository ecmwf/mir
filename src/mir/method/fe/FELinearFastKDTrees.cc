/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/method/fe/FELinearFastKDTrees.h"

#include <algorithm>
#include <forward_list>
#include <limits>
#include <utility>
#include "eckit/config/Resource.h"
#include "eckit/log/BigNum.h"
#include "eckit/log/Plural.h"
#include "eckit/log/ProgressTimer.h"
#include "eckit/log/ResourceUsage.h"
#include "eckit/log/TraceTimer.h"
#include "eckit/memory/ScopedPtr.h"
#include "eckit/utils/MD5.h"
#include "atlas/interpolation/element/Quad3D.h"
#include "atlas/interpolation/element/Triag3D.h"
#include "atlas/interpolation/method/PointIndex3.h"
#include "atlas/interpolation/method/Ray.h"
#include "atlas/mesh/ElementType.h"
#include "atlas/mesh/Elements.h"
#include "atlas/mesh/Nodes.h"
#include "atlas/output/Gmsh.h"
#include "mir/config/LibMir.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Iterator.h"
#include "mir/repres/Representation.h"
#include "mir/util/PointSearch.h"


namespace mir {
namespace method {
namespace fe {


namespace {


static MethodBuilder< FELinearFastKDTrees > __linear_fast_kdtrees("linear-fast-kdtrees");


// epsilon used to scale edge tolerance when projecting ray to intesect element
static const double parametricEpsilon = 1e-15;


typedef std::vector< size_t > element_indices_t;
typedef std::vector< WeightMatrix::Triplet > triplet_vector_t;
typedef atlas::interpolation::method::ElemIndex3 element_tree_t;
typedef std::pair< size_t, repres::Iterator::point_ll_t > failed_projection_t;


static void normalise(triplet_vector_t& triplets) {

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
    const element_indices_t& closestElements,
    size_t nbInputPoints,
    const atlas::array::ArrayView<double, 2>& icoords,
    const atlas::mesh::HybridElements::Connectivity& elementsToNodes,
    const repres::Iterator::point_3d_t& p,
    size_t ip,
    size_t firstVirtualPoint,
    size_t& nbProjectionAttempts) {

    triplet_vector_t triplets;

    bool mustNormalise = false;
    size_t idx[4];
    double w[4];
    atlas::interpolation::method::Ray ray( p.data() );    


    nbProjectionAttempts = 0;
    for (const size_t& elem_id : closestElements) {
        ++nbProjectionAttempts;

        ASSERT(elem_id < elementsToNodes.rows());

        /* assumes:
         * - nb_cols == 3 implies triangle
         * - nb_cols == 4 implies quadrilateral
         * - no other element is supported at the time
         */
        const size_t nb_cols = elementsToNodes.cols(elem_id);
        ASSERT(nb_cols == 3 || nb_cols == 4);

        for (size_t i = 0; i < nb_cols; ++i) {
            idx[i] = size_t(elementsToNodes(elem_id, i));
            ASSERT(idx[i] < nbInputPoints);
        }

        if (nb_cols == 3) {

            /* triangle */
            atlas::interpolation::element::Triag3D triag(
                atlas::PointXYZ{ icoords(idx[0], 0), icoords(idx[0], 1), icoords(idx[0], 2) },
                atlas::PointXYZ{ icoords(idx[1], 0), icoords(idx[1], 1), icoords(idx[1], 2) },
                atlas::PointXYZ{ icoords(idx[2], 0), icoords(idx[2], 1), icoords(idx[2], 2) });

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
                    if (idx[i] < firstVirtualPoint)
                        triplets.push_back( WeightMatrix::Triplet( ip, idx[i], w[i] ) );
                    else
                        mustNormalise = true;
                }

                break; // stop looking for elements
            }

        } else {

            /* quadrilateral */
            atlas::interpolation::element::Quad3D quad(
                atlas::PointXYZ{ icoords(idx[0], 0), icoords(idx[0], 1), icoords(idx[0], 2) },
                atlas::PointXYZ{ icoords(idx[1], 0), icoords(idx[1], 1), icoords(idx[1], 2) },
                atlas::PointXYZ{ icoords(idx[2], 0), icoords(idx[2], 1), icoords(idx[2], 2) },
                atlas::PointXYZ{ icoords(idx[3], 0), icoords(idx[3], 1), icoords(idx[3], 2) });

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
                    if (idx[i] < firstVirtualPoint)
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


void FELinearFastKDTrees::assemble(util::MIRStatistics& statistics,
                                   WeightMatrix& W,
                                   const repres::Representation& in,
                                   const repres::Representation& out) const {
    eckit::Log::debug<LibMir>() << "FiniteElement::assemble (input: " << in << ", output: " << out << ")" << std::endl;

    auto inputMeshGenerationParams = inputMeshGenerationParams_;
    auto outputMeshGenerationParams = outputMeshGenerationParams_;

    // let representations set the mesh generator
    if (inputMeshGenerationParams.meshGenerator_ == "") {
        inputMeshGenerationParams.meshGenerator_ = in.atlasMeshGenerator();
    }

    if (outputMeshGenerationParams.meshGenerator_ == "") {
        outputMeshGenerationParams.meshGenerator_ = out.atlasMeshGenerator();
    }


    // get input mesh (cell centres are required for the k-d tree)
    ASSERT(inputMeshGenerationParams.meshCellCentres_);
    util::MIRGrid gin(in.atlasGrid());
    const atlas::Mesh& inMesh = gin.mesh(statistics, inputMeshGenerationParams);
    const util::Domain& inDomain = in.domain();

    const atlas::mesh::Nodes& inNodes = inMesh.nodes();
    atlas::array::ArrayView<double, 2> icoords = atlas::array::make_view< double, 2 >( inNodes.field( "xyz" ));

    size_t firstVirtualPoint = std::numeric_limits<size_t>::max();
    if (inNodes.metadata().has("NbRealPts")) {
        firstVirtualPoint = inNodes.metadata().get<size_t>("NbRealPts");
    }


    // generate nodes-to-elements/elements-to-nodes connectivity
    const atlas::mesh::HybridElements::Connectivity& elementsToNodes = inMesh.cells().node_connectivity();
    std::vector< element_indices_t > nodesToElements(inNodes.size());
    {
        const size_t Nnodes = inNodes.size();
        const size_t Nelems = inMesh.cells().size();

        eckit::ProgressTimer progress("Building reverse connectivity", Nelems, "element", double(1), eckit::Log::debug<LibMir>());

        auto patched = atlas::array::make_view< int, 1 >(inMesh.cells().field("patch"));
        for (size_t e = 0; e < Nelems; ++e, ++progress) {
            for (size_t i = 0; not patched(e) and i < elementsToNodes.cols(e); ++i) {
                size_t n = elementsToNodes(e, i);
                ASSERT(n < Nnodes);
                nodesToElements[n].push_back(e);
            }
        }

#if 0
        for (auto& elems : nodesToElements) {
            std::set<size_t> unique(elems.cbegin(), elems.cend());
            elems.assign(unique.cbegin(), unique.cend());
        }
#endif
    }


    // generate k-d tree with node indices
    const util::PointSearch sptree(parametrisation_, in);

    double R = 0.;
    if (!in.getLongestElementDiagonal(R)) {
        R = gin.getMeshLongestElementDiagonal();
    }
    ASSERT(R > 0.);
    eckit::Log::debug<LibMir>() << "k-d tree: search radius R=" << eckit::BigNum(static_cast<long long>(R)) << "m" << std::endl;


    // some statistics
    const size_t nbInputPoints = inNodes.size();
    const size_t nbOutputPoints = out.numberOfPoints();
    size_t nbMaxProjectionAttempts = 0;
    size_t nbProjections = 0;
    size_t nbFailures = 0;
    std::forward_list<failed_projection_t> failures;


    // weights -- one per vertex of element, triangles (3) or quads (4)
    triplet_vector_t weights_triplets; // structure to fill-in sparse matrix
    weights_triplets.reserve( nbOutputPoints * 4 );        // preallocate space as if all elements where quads

    {
        eckit::ProgressTimer progress("Projecting", nbOutputPoints, "point", double(5), eckit::Log::debug<LibMir>());

        // output points
        const eckit::ScopedPtr<repres::Iterator> it(out.iterator());
        size_t ip = 0;

        while (it->next()) {
            ASSERT(ip < nbOutputPoints);
            ++progress;

            if (inDomain.contains(it->pointUnrotated())) {

                // 3D point to lookup
                repres::Iterator::point_3d_t p(it->point3D());

                const util::PointSearch::PointValueType closest = sptree.closestPoint(p);
                ASSERT(R > util::PointSearch::PointType::distance(closest.point(), p));

                size_t n = closest.payload();
                ASSERT(n < nbInputPoints);

                ASSERT(nodesToElements[n].size() < 12);

                size_t nbProjectionAttempts;
                triplet_vector_t triplets = projectPointTo3DElements(
                            nodesToElements[n],
                            nbInputPoints,
                            icoords,
                            elementsToNodes,
                            p,
                            ip,
                            firstVirtualPoint,
                            nbProjectionAttempts );

                nbMaxProjectionAttempts = std::max(nbMaxProjectionAttempts, nbProjectionAttempts);

                if (triplets.empty()) {
                    // If this fails, consider lowering parametricEpsilon
                    failures.push_front(failed_projection_t(ip, it->pointUnrotated()));
                    ++nbFailures;
                } else {
                    std::copy(triplets.begin(), triplets.end(), std::back_inserter(weights_triplets));
                    ++nbProjections;
                }
            }

            ++ip;
        }
    }

    eckit::Log::debug<LibMir>()
            << "Projected " << eckit::BigNum(nbProjections)
            << " of " << eckit::Plural(nbOutputPoints, "point")
            << " (" << eckit::Plural(nbFailures, "failure") << ")\n"
            << "k-d tree: up to " << eckit::Plural(nbMaxProjectionAttempts, "projection attempt") << " (per point)"
            << std::endl;

    if (nbFailures) {
        std::stringstream msg;
        msg << "Failed to project " << eckit::Plural(nbFailures, "point");
        eckit::Log::debug<LibMir>() << msg.str() << ":";
        size_t count = 0;
        for (const failed_projection_t& f : failures) {
            eckit::Log::debug<LibMir>() << "\n\tpoint " << f.first << " " << f.second;
//            if (++count > 10) {
//                eckit::Log::debug<LibMir>() << "\n\t...";
//                break;
//            }
        }
        eckit::Log::debug<LibMir>() << std::endl;
        throw eckit::SeriousBug(msg.str());
    }


    // fill sparse matrix
    W.setFromTriplets(weights_triplets);
}


}  // namespace fe
}  // namespace method
}  // namespace mir

