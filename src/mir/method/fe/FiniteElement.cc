/*
 * (C) Copyright 1996- ECMWF.
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


#include "mir/method/fe/FiniteElement.h"

#include <algorithm>
#include <forward_list>
#include <limits>
#include <memory>
#include <utility>

#include "eckit/config/Resource.h"
#include "eckit/log/BigNum.h"
#include "eckit/log/Plural.h"
#include "eckit/log/ProgressTimer.h"
#include "eckit/log/ResourceUsage.h"
#include "eckit/log/Timer.h"
#include "eckit/types/FloatCompare.h"
#include "eckit/utils/MD5.h"

#include "atlas/interpolation/element/Quad3D.h"
#include "atlas/interpolation/element/Triag3D.h"
#include "atlas/interpolation/method/PointIndex3.h"
#include "atlas/interpolation/method/Ray.h"
#include "atlas/mesh/ElementType.h"
#include "atlas/mesh/Elements.h"
#include "atlas/mesh/Nodes.h"

#include "mir/config/LibMir.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Iterator.h"
#include "mir/repres/Representation.h"


namespace mir {
namespace method {
namespace fe {


namespace {


// epsilon used to scale edge tolerance when projecting ray to intesect element
static const double parametricEpsilon = 1e-15;


using triplet_vector_t = std::vector< WeightMatrix::Triplet >;
using element_tree_t = atlas::interpolation::method::ElemIndex3;
using failed_projection_t = std::pair< size_t, PointLatLon >;


static void normalise(triplet_vector_t& triplets) {
    ASSERT(!triplets.empty());

    // sum all calculated weights for normalisation
    double sum = 0.;
    for (auto& t : triplets) {
        sum += t.value();
    }

    if (sum > std::numeric_limits<double>::epsilon()) {

        // now normalise all weights according to the total
        const double invSum = 1. / sum;
        for (auto& t : triplets) {
            t.value() *= invSum;
        }

    } else {

        // if no reasonable seight sum is found, distribute equitably
        const double invSum = 1. / triplets.size();
        for (auto& t : triplets) {
            t.value() = invSum;
        }

    }
}


/// Find in which element the point is contained by projecting the point with each nearest element
static triplet_vector_t projectPointTo3DElements(
    size_t nbInputPoints,
    const atlas::array::ArrayView<double, 2>& icoords,
    const atlas::mesh::HybridElements::Connectivity& connectivity,
    const Point3& p,
    size_t ip,
    size_t firstVirtualPoint,
    size_t& nbProjectionAttempts,
    const element_tree_t::NodeList& closest ) {

    if (closest.empty()) {
        return {};
    }

    triplet_vector_t triplets;

    bool mustNormalise = false;
    size_t idx[4];
    double w[4];
    atlas::interpolation::method::Ray ray( p.data() );

    nbProjectionAttempts = 0;
    for (const auto& close : closest) {
        ++nbProjectionAttempts;

        const size_t elem_id = close.value().payload();
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
                    if (idx[i] < firstVirtualPoint) {
                        triplets.push_back(WeightMatrix::Triplet(ip, idx[i], w[i]));
                    }
                    else {
                        mustNormalise = true;
                    }
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
                    if (idx[i] < firstVirtualPoint) {
                        triplets.push_back(WeightMatrix::Triplet(ip, idx[i], w[i]));
                    }
                    else {
                        mustNormalise = true;
                    }
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


FiniteElement::FiniteElement(const param::MIRParametrisation& param) :
    MethodWeighted(param),
    inputMeshGenerationParams_("input", param) {

    // input mesh requirements
    inputMeshGenerationParams_.meshCellCentres_ = true;
}


FiniteElement::~FiniteElement() = default;


void FiniteElement::print(std::ostream &out) const {
    MethodWeighted::print(out);
}


bool FiniteElement::sameAs(const Method& other) const {
    auto o = dynamic_cast<const FiniteElement*>(&other);
    return o
            && inputMeshGenerationParams_.sameAs(o->inputMeshGenerationParams_)
            && MethodWeighted::sameAs(other);
}


void FiniteElement::hash(eckit::MD5& md5) const {
    MethodWeighted::hash(md5);
    inputMeshGenerationParams_.hash(md5);
}


void FiniteElement::assemble(util::MIRStatistics& statistics,
                             WeightMatrix& W,
                             const repres::Representation& in,
                             const repres::Representation& out) const {
    eckit::Channel& log = eckit::Log::debug<LibMir>();

    log << "FiniteElement::assemble (input: " << in << ", output: " << out << ")" << std::endl;

    // let representation set the mesh generator parameters
    auto inputMeshGenerationParams = inputMeshGenerationParams_;
    in.fill(inputMeshGenerationParams);
    ASSERT(inputMeshGenerationParams.meshCellCentres_);  // required for the k-d tree


    // get input mesh
    util::MIRGrid gin(in.atlasGrid());
    const atlas::Mesh& inMesh = gin.mesh(statistics, inputMeshGenerationParams);
    const util::Domain& inDomain = in.domain();

    const atlas::mesh::Nodes& inNodes = inMesh.nodes();
    atlas::array::ArrayView<double, 2> icoords = atlas::array::make_view< double, 2 >( inNodes.field( "xyz" ));

    size_t firstVirtualPoint = std::numeric_limits<size_t>::max();
    if (inNodes.metadata().has("NbRealPts")) {
        firstVirtualPoint = inNodes.metadata().get<size_t>("NbRealPts");
    }


    // generate k-d tree with cell centres
    std::unique_ptr<element_tree_t> eTree;
    {
        eckit::ResourceUsage usage("FiniteElement::assemble create k-d tree", log);
        eckit::TraceTimer<LibMir> timer("k-d tree: create");
        eTree.reset( atlas::interpolation::method::create_element_centre_index(inMesh) );
    }

    double R = 0.;
    if (!in.getLongestElementDiagonal(R)) {
        R = gin.getMeshLongestElementDiagonal();
    }
    ASSERT(R > 0.);
    log << "k-d tree: search radius R=" << eckit::BigNum(static_cast<long long>(R)) << "m" << std::endl;


    // some statistics
    const size_t nbInputPoints = inNodes.size();
    const size_t nbOutputPoints = out.numberOfPoints();
    size_t nbMinElementsSearched = std::numeric_limits<size_t>::max();
    size_t nbMaxElementsSearched = 0;
    size_t nbMaxProjectionAttempts = 0;
    size_t nbProjections = 0;

    size_t nbFailures = 0;
    std::forward_list<failed_projection_t> failures;
    bool failuresAreMissingValues = !inDomain.isGlobal();


    // weights -- one per vertex of element, triangles (3) or quads (4)
    triplet_vector_t weights_triplets; // structure to fill-in sparse matrix
    weights_triplets.reserve( nbOutputPoints * 4 );        // preallocate space as if all elements where quads

    {
        eckit::ProgressTimer progress("Projecting", nbOutputPoints, "point", double(5), log);

        const atlas::mesh::HybridElements::Connectivity& connectivity = inMesh.cells().node_connectivity();


        // output points
        const std::unique_ptr<repres::Iterator> it(out.iterator());
        size_t ip = 0;

        while (it->next()) {
            ASSERT(ip < nbOutputPoints);
            ++progress;

            if (inDomain.contains(it->pointRotated())) {

                // 3D point to lookup
                Point3 p(it->point3D());

                // 3D projection, trying elements closest to p first
                element_tree_t::NodeList closest = eTree->findInSphere(p, R);

                size_t nbProjectionAttempts = 0;
                triplet_vector_t triplets = projectPointTo3DElements(
                                                nbInputPoints,
                                                icoords,
                                                connectivity,
                                                p,
                                                ip,
                                                firstVirtualPoint,
                                                nbProjectionAttempts,
                                                closest );

                nbMaxElementsSearched = std::max(nbMaxElementsSearched, closest.size());
                nbMinElementsSearched = std::min(nbMinElementsSearched, closest.size());
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

    log << "Projected " << eckit::BigNum(nbProjections) << " of "
        << eckit::Plural(nbOutputPoints, "point") << " (" << eckit::Plural(nbFailures, "failure")
        << ")\n"
        << "k-d tree: searched between " << eckit::BigNum(nbMinElementsSearched) << " and "
        << eckit::Plural(nbMaxElementsSearched, "element") << ", with up to "
        << eckit::Plural(nbMaxProjectionAttempts, "projection attempt") << " (per point)"
        << std::endl;

    if (nbFailures && !failuresAreMissingValues) {
        std::stringstream msg;
        msg << "Failed to project " << eckit::Plural(nbFailures, "point");
        log << msg.str() << ":";
        size_t count = 0;
        for (const auto& f : failures) {
            log << "\n\tpoint " << f.first << " " << f.second;
            if (++count > 10) {
                log << "\n\t...";
                break;
            }
        }
        log << std::endl;
        throw eckit::SeriousBug(msg.str());
    }


    // fill sparse matrix
    W.setFromTriplets(weights_triplets);
}


}  // namespace fe
}  // namespace method
}  // namespace mir

