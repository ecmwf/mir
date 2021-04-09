/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 *
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/method/fe/FiniteElement.h"

#include <algorithm>
#include <cmath>
#include <forward_list>
#include <iterator>
#include <limits>
#include <map>
#include <memory>
#include <mutex>
#include <ostream>
#include <sstream>
#include <utility>

#include "eckit/utils/MD5.h"
#include "eckit/utils/StringTools.h"

#include "mir/caching/InMemoryMeshCache.h"
#include "mir/repres/Iterator.h"
#include "mir/repres/Representation.h"
#include "mir/util/Domain.h"
#include "mir/util/Log.h"
#include "mir/util/Trace.h"


namespace mir {
namespace method {
namespace fe {


// epsilon used to scale edge tolerance when projecting ray to intesect element
static const double parametricEpsilon = 1e-15;

static std::once_flag once;
static std::recursive_mutex* mtx                       = nullptr;
static std::map<std::string, FiniteElementFactory*>* m = nullptr;
static void init() {
    mtx = new std::recursive_mutex();
    m   = new std::map<std::string, FiniteElementFactory*>();
}

using triplet_vector_t    = std::vector<WeightMatrix::Triplet>;
using element_tree_t      = atlas::interpolation::method::ElemIndex3;
using failed_projection_t = std::pair<size_t, PointLatLon>;


static void normalise(triplet_vector_t& triplets) {
    ASSERT(!triplets.empty());

    // sum all calculated weights for normalisation
    double sum = 0.;
    for (auto& t : triplets) {
        sum += t.value();
    }

    // normalise all weights according to the total
    if (sum > std::numeric_limits<double>::epsilon()) {
        const double invSum = 1. / sum;
        for (auto& t : triplets) {
            t.value() *= invSum;
        }
        return;
    }

    // if no reasonable weight sum is found, distribute equitably
    const double invSum = 1. / double(triplets.size());
    for (auto& t : triplets) {
        t.value() = invSum;
    }
}


/// Find in which element the point is contained by projecting the point with each nearest element
static triplet_vector_t projectPointTo3DElements(size_t nbInputPoints,
                                                 const atlas::array::ArrayView<double, 2>& icoords,
                                                 const atlas::mesh::HybridElements::Connectivity& connectivity,
                                                 const Point3& p, size_t ip, size_t firstVirtualPoint,
                                                 size_t& nbProjectionAttempts,
                                                 const element_tree_t::NodeList& closest) {

    if (closest.empty()) {
        return {};
    }

    triplet_vector_t triplets;

    bool mustNormalise = false;
    size_t idx[4];
    double w[4];
    constexpr size_t XX = 0;
    constexpr size_t YY = 1;
    constexpr size_t ZZ = 2;

    atlas::interpolation::method::Ray ray(p.data());

    nbProjectionAttempts = 0;
    for (const auto& close : closest) {
        ++nbProjectionAttempts;

        const auto elem_id = atlas::idx_t(close.value().payload());
        ASSERT(elem_id < connectivity.rows());

        /* assumes:
         * - nb_cols == 3 implies triangle
         * - nb_cols == 4 implies quadrilateral
         * - no other element is supported at the time
         */
        const auto nb_cols = connectivity.cols(elem_id);
        ASSERT(nb_cols == 3 || nb_cols == 4);

        for (atlas::idx_t i = 0; i < nb_cols; ++i) {
            idx[i] = size_t(connectivity(elem_id, i));
            ASSERT(idx[i] < nbInputPoints);
        }

        if (nb_cols == 3) {

            /* triangle */
            atlas::interpolation::element::Triag3D triag(
                atlas::PointXYZ{icoords(idx[0], XX), icoords(idx[0], YY), icoords(idx[0], ZZ)},
                atlas::PointXYZ{icoords(idx[1], XX), icoords(idx[1], YY), icoords(idx[1], ZZ)},
                atlas::PointXYZ{icoords(idx[2], XX), icoords(idx[2], YY), icoords(idx[2], ZZ)});

            // pick an epsilon based on a characteristic length (sqrt(area))
            // (this scales linearly so it better compares with linear weights u,v,w)
            const double edgeEpsilon = parametricEpsilon * std::sqrt(triag.area());
            ASSERT(edgeEpsilon >= 0);

            auto is = triag.intersects(ray, edgeEpsilon);
            if (is) {

                // weights are the linear Lagrange function evaluated at u,v (aka barycentric coordinates)
                w[0] = 1. - is.u - is.v;
                w[1] = is.u;
                w[2] = is.v;

                for (size_t i = 0; i < 3; ++i) {
                    if (idx[i] < firstVirtualPoint) {
                        triplets.push_back(WeightMatrix::Triplet(ip, idx[i], w[i]));
                    }
                    else {
                        mustNormalise = true;
                    }
                }

                break;  // stop looking for elements
            }
        }
        else {

            /* quadrilateral */
            atlas::interpolation::element::Quad3D quad(
                atlas::PointXYZ{icoords(idx[0], XX), icoords(idx[0], YY), icoords(idx[0], ZZ)},
                atlas::PointXYZ{icoords(idx[1], XX), icoords(idx[1], YY), icoords(idx[1], ZZ)},
                atlas::PointXYZ{icoords(idx[2], XX), icoords(idx[2], YY), icoords(idx[2], ZZ)},
                atlas::PointXYZ{icoords(idx[3], XX), icoords(idx[3], YY), icoords(idx[3], ZZ)});

            if (!quad.validate()) {  // somewhat expensive sanity check
                Log::warning() << "Invalid Quad : " << quad << std::endl;
                throw exception::SeriousBug("Found invalid quadrilateral in mesh", Here());
            }

            // pick an epsilon based on a characteristic length (sqrt(area))
            // (this scales linearly so it better compares with linear weights u,v,w)
            const double edgeEpsilon = parametricEpsilon * std::sqrt(quad.area());
            ASSERT(edgeEpsilon >= 0);

            auto is = quad.intersects(ray, edgeEpsilon);
            if (is) {

                // weights are the bilinear Lagrange function evaluated at u,v
                w[0] = (1. - is.u) * (1. - is.v);
                w[1] = is.u * (1. - is.v);
                w[2] = is.u * is.v;
                w[3] = (1. - is.u) * is.v;


                for (size_t i = 0; i < 4; ++i) {
                    if (idx[i] < firstVirtualPoint) {
                        triplets.push_back(WeightMatrix::Triplet(ip, idx[i], w[i]));
                    }
                    else {
                        mustNormalise = true;
                    }
                }

                break;  // stop looking for elements
            }
        }

    }  // loop over nearest elements

    // at least one of the nodes of element shouldn't be virtual
    if (!triplets.empty() && mustNormalise) {
        normalise(triplets);
    }

    return triplets;
}


FiniteElement::FiniteElement(const param::MIRParametrisation& param, const std::string& label) :
    MethodWeighted(param), meshGeneratorParams_(param, label) {

    // input mesh requirements
    meshGeneratorParams_.meshCellCentres_ = true;
}


atlas::Mesh FiniteElement::atlasMesh(util::MIRStatistics& statistics, const repres::Representation& repres) const {

    // let representation set mesh generator parameters
    auto params = meshGeneratorParams_;
    repres.fill(params);

    double d;
    if (!repres.getLongestElementDiagonal(d)) {
        params.meshCellLongestDiagonal_ = true;
    }

    auto msh = caching::InMemoryMeshCache::atlasMesh(statistics, repres.atlasGrid(), params);
    if (!params.meshCellLongestDiagonal_) {
        ASSERT(d > 0.);
        msh.metadata().set("cell_longest_diagonal", d);
    }
    return msh;
}


FiniteElement::~FiniteElement() = default;


void FiniteElement::print(std::ostream& out) const {
    out << "FiniteElement[name=" << name() << ",";
    MethodWeighted::print(out);
    out << "]";
}


bool FiniteElement::sameAs(const Method& other) const {
    auto o = dynamic_cast<const FiniteElement*>(&other);
    return (o != nullptr) && meshGeneratorParams_.sameAs(o->meshGeneratorParams_) && MethodWeighted::sameAs(other);
}


void FiniteElement::hash(eckit::MD5& md5) const {
    MethodWeighted::hash(md5);
    meshGeneratorParams_.hash(md5);
}


void FiniteElement::assemble(util::MIRStatistics& statistics, WeightMatrix& W, const repres::Representation& in,
                             const repres::Representation& out) const {
    auto& log = Log::debug();

    log << "FiniteElement::assemble (input: " << in << ", output: " << out << ")" << std::endl;


    // get input mesh
    ASSERT(meshGeneratorParams().meshCellCentres_);  // required for the k-d tree

    const auto& inMesh   = atlasMesh(statistics, in);
    const auto& inDomain = in.domain();

    auto& inNodes = inMesh.nodes();
    auto icoords  = atlas::array::make_view<double, 2>(inNodes.field("xyz"));

    size_t firstVirtualPoint = std::numeric_limits<size_t>::max();
    if (inNodes.metadata().has("NbRealPts")) {
        firstVirtualPoint = inNodes.metadata().get<size_t>("NbRealPts");
    }


    // generate k-d tree with cell centres
    std::unique_ptr<element_tree_t> eTree;
    {
        trace::ResourceUsage timer("k-d tree: create");
        eTree.reset(atlas::interpolation::method::create_element_centre_index(inMesh));
    }

    double R = inMesh.metadata().getDouble("cell_longest_diagonal");
    ASSERT(R > 0.);
    log << "k-d tree: search radius R=" << R << "m" << std::endl;


    // some statistics
    const auto nbInputPoints       = size_t(inNodes.size());
    const size_t nbOutputPoints    = out.numberOfPoints();
    size_t nbMinElementsSearched   = std::numeric_limits<size_t>::max();
    size_t nbMaxElementsSearched   = 0;
    size_t nbMaxProjectionAttempts = 0;
    size_t nbProjections           = 0;

    size_t nbFailures = 0;
    std::forward_list<failed_projection_t> failures;
    bool failuresAreMissingValues = !inDomain.isGlobal();


    // weights -- one per vertex of element, triangles (3) or quads (4)
    triplet_vector_t weights_triplets;             // structure to fill-in sparse matrix
    weights_triplets.reserve(nbOutputPoints * 4);  // preallocate space as if all elements where quads

    {
        trace::ProgressTimer progress("Projecting", nbOutputPoints, {"point"}, log);

        auto& connectivity = inMesh.cells().node_connectivity();


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
                size_t nbProjectionAttempts = 0;

                auto closest  = eTree->findInSphere(p, R);
                auto triplets = projectPointTo3DElements(nbInputPoints, icoords, connectivity, p, ip, firstVirtualPoint,
                                                         nbProjectionAttempts, closest);

                nbMaxElementsSearched   = std::max(nbMaxElementsSearched, closest.size());
                nbMinElementsSearched   = std::min(nbMinElementsSearched, closest.size());
                nbMaxProjectionAttempts = std::max(nbMaxProjectionAttempts, nbProjectionAttempts);

                if (triplets.empty()) {
                    // If this fails, consider lowering parametricEpsilon
                    failures.emplace_front(ip, it->pointUnrotated());
                    ++nbFailures;
                }
                else {
                    std::copy(triplets.begin(), triplets.end(), std::back_inserter(weights_triplets));
                    ++nbProjections;
                }
            }

            ++ip;
        }
    }

    log << "Projected " << Log::Pretty(nbProjections) << " of " << Log::Pretty(nbOutputPoints, {"point"}) << " ("
        << Log::Pretty(nbFailures, {"failure"}) << ")\n"
        << "k-d tree: searched between " << Log::Pretty(nbMinElementsSearched) << " and "
        << Log::Pretty(nbMaxElementsSearched, {"element"}) << ", with up to "
        << Log::Pretty(nbMaxProjectionAttempts, {"projection attempt"}) << " (per point)" << std::endl;

    if ((nbFailures > 0) && !failuresAreMissingValues) {
        std::stringstream msg;
        msg << "Failed to project " << Log::Pretty(nbFailures, {"point"});
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
        throw exception::SeriousBug(msg.str());
    }


    // fill sparse matrix
    W.setFromTriplets(weights_triplets);
}


FiniteElementFactory::FiniteElementFactory(const std::string& name) : MethodFactory(name), name_(name) {
    std::call_once(once, init);
    std::lock_guard<std::recursive_mutex> guard(*mtx);

    if (m->find(name) != m->end()) {
        throw exception::SeriousBug("FiniteElementFactory: duplicate '" + name + "'");
    }

    ASSERT(m->find(name) == m->end());
    (*m)[name] = this;
}


FiniteElementFactory::~FiniteElementFactory() {
    std::call_once(once, init);
    std::lock_guard<std::recursive_mutex> guard(*mtx);

    m->erase(name_);
}


void FiniteElementFactory::list(std::ostream& out) {
    std::call_once(once, init);
    std::lock_guard<std::recursive_mutex> guard(*mtx);

    const char* sep = "";
    for (const auto& j : *m) {
        out << sep << j.first;
        sep = ", ";
    }
}


FiniteElement* FiniteElementFactory::build(std::string& names, const std::string& label,
                                           const param::MIRParametrisation& param) {
    std::call_once(once, init);
    std::lock_guard<std::recursive_mutex> guard(*mtx);

    for (const auto& name : eckit::StringTools::split("/", names)) {
        Log::debug() << "FiniteElementFactory: looking for '" << name << "'" << std::endl;
        auto j = m->find(name);
        if (j != m->end()) {
            names = name;
            return j->second->make(param, label);
        }
    }

    list(Log::error() << "FiniteElementFactory: unknown '" << names << "', choices are: ");
    throw exception::SeriousBug("FiniteElementFactory: unknown '" + names + "'");
}


}  // namespace fe
}  // namespace method
}  // namespace mir
