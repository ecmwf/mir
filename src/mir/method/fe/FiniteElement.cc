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
#include <forward_list>
#include <limits>
#include <map>
#include <memory>
#include <utility>

#include "eckit/config/Resource.h"
#include "eckit/log/ResourceUsage.h"
#include "eckit/log/TraceTimer.h"
#include "eckit/thread/AutoLock.h"
#include "eckit/thread/Mutex.h"
#include "eckit/utils/MD5.h"

#include "mir/api/Atlas.h"

#include "mir/caching/InMemoryCache.h"
#include "mir/config/LibMir.h"
#include "mir/method/fe/BuildNodeLumpedMassMatrix.h"
#include "mir/method/fe/CalculateCellLongestDiagonal.h"
#include "mir/repres/Iterator.h"
#include "mir/repres/Representation.h"
#include "mir/util/Domain.h"
#include "mir/util/MIRStatistics.h"
#include "mir/util/Pretty.h"


namespace mir {
namespace method {
namespace fe {


// epsilon used to scale edge tolerance when projecting ray to intesect element
static const double parametricEpsilon = 1e-15;

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

    if (sum > std::numeric_limits<double>::epsilon()) {

        // now normalise all weights according to the total
        const double invSum = 1. / sum;
        for (auto& t : triplets) {
            t.value() *= invSum;
        }
    }
    else {

        // if no reasonable seight sum is found, distribute equitably
        const double invSum = 1. / triplets.size();
        for (auto& t : triplets) {
            t.value() = invSum;
        }
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
                atlas::PointXYZ{icoords(idx[0], 0), icoords(idx[0], 1), icoords(idx[0], 2)},
                atlas::PointXYZ{icoords(idx[1], 0), icoords(idx[1], 1), icoords(idx[1], 2)},
                atlas::PointXYZ{icoords(idx[2], 0), icoords(idx[2], 1), icoords(idx[2], 2)});

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
                atlas::PointXYZ{icoords(idx[0], 0), icoords(idx[0], 1), icoords(idx[0], 2)},
                atlas::PointXYZ{icoords(idx[1], 0), icoords(idx[1], 1), icoords(idx[1], 2)},
                atlas::PointXYZ{icoords(idx[2], 0), icoords(idx[2], 1), icoords(idx[2], 2)},
                atlas::PointXYZ{icoords(idx[3], 0), icoords(idx[3], 1), icoords(idx[3], 2)});

            if (!quad.validate()) {  // somewhat expensive sanity check
                eckit::Log::warning() << "Invalid Quad : " << quad << std::endl;
                throw eckit::SeriousBug("Found invalid quadrilateral in mesh", Here());
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


static caching::InMemoryCache<atlas::Mesh> mesh_cache("mirMesh", 512 * 1024 * 1024, 0,
                                                      "$MIR_MESH_CACHE_MEMORY_FOOTPRINT");


static pthread_once_t once                             = PTHREAD_ONCE_INIT;
static eckit::Mutex* local_mutex                       = nullptr;
static std::map<std::string, FiniteElementFactory*>* m = nullptr;
static void init() {
    local_mutex = new eckit::Mutex();
    m           = new std::map<std::string, FiniteElementFactory*>();
}


FiniteElement::FiniteElement(const param::MIRParametrisation& param, const std::string& label) :
    MethodWeighted(param),
    meshGeneratorParams_(label, param) {

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

    auto msh = atlasMesh(statistics, repres.atlasGrid(), params);
    if (!params.meshCellLongestDiagonal_) {
        ASSERT(d > 0.);
        msh.metadata().set("cell_longest_diagonal", d);
    }
    return msh;
}


atlas::Mesh FiniteElement::atlasMesh(util::MIRStatistics& statistics, const atlas::Grid& grid,
                                     const util::MeshGeneratorParameters& meshGeneratorParams) const {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    eckit::Channel& log = eckit::Log::debug<LibMir>();

    eckit::ResourceUsage usage_mesh("Mesh for grid " + grid.name() + " (" + grid.uid() + ")", log);
    caching::InMemoryCacheUser<atlas::Mesh> cache_use(mesh_cache, statistics.meshCache_);

    // generate signature including the mesh generation settings
    eckit::MD5 md5;
    md5 << grid;
    md5 << meshGeneratorParams;

    auto j = mesh_cache.find(md5);
    if (j != mesh_cache.end()) {
        return *j;
    }

    atlas::Mesh& mesh = mesh_cache[md5];
    ASSERT(!mesh.generated());

    try {

        const std::string meshGenerator = meshGeneratorParams.meshGenerator_;
        if (meshGenerator.empty()) {
            throw eckit::SeriousBug("Mesh: no mesh generator defined ('" + meshGenerator + "')");
        }

        atlas::MeshGenerator generator(meshGenerator, meshGeneratorParams);
        mesh = generator.generate(grid);
        ASSERT(mesh.generated());

        // If meshgenerator did not create xyz field already, do it now.
        {
            eckit::ResourceUsage usage("BuildXYZField", log);
            eckit::TraceTimer<LibMir> timer("Mesh: BuildXYZField");
            atlas::mesh::actions::BuildXYZField()(mesh);
        }

        // Calculate barycenters of mesh cells
        if (meshGeneratorParams.meshCellCentres_) {
            eckit::ResourceUsage usage("BuildCellCentres", log);
            eckit::TraceTimer<LibMir> timer("Mesh: BuildCellCentres");
            atlas::mesh::actions::BuildCellCentres()(mesh);
        }

        // Calculate the mesh cells longest diagonal
        if (meshGeneratorParams.meshCellLongestDiagonal_) {
            eckit::ResourceUsage usage("CalculateCellLongestDiagonal", log);
            eckit::TraceTimer<LibMir> timer("Mesh: CalculateCellLongestDiagonal");
            CalculateCellLongestDiagonal()(mesh);
        }

        // Calculate node-lumped mass matrix
        if (meshGeneratorParams.meshNodeLumpedMassMatrix_) {
            eckit::ResourceUsage usage("BuildNodeLumpedMassMatrix", log);
            eckit::TraceTimer<LibMir> timer("Mesh: BuildNodeLumpedMassMatrix");
            BuildNodeLumpedMassMatrix()(mesh);
        }

        // Calculate node-to-cell ("inverse") connectivity
        if (meshGeneratorParams.meshNodeToCellConnectivity_) {
            eckit::ResourceUsage usage("BuildNode2CellConnectivity", log);
            eckit::TraceTimer<LibMir> timer("Mesh: BuildNode2CellConnectivity");
            atlas::mesh::actions::BuildNode2CellConnectivity{mesh}();
        }

        // Some information
        log << "Mesh["
               "cells="
            << Pretty(mesh.cells().size()) << ",nodes=" << Pretty(mesh.nodes().size()) << "," << meshGeneratorParams
            << "]" << std::endl;

        // Write file(s)
        if (!meshGeneratorParams.fileLonLat_.empty()) {
            atlas::output::PathName path(meshGeneratorParams.fileLonLat_);
            log << "Mesh: writing to '" << path << "'" << std::endl;
            atlas::output::Gmsh(path, atlas::util::Config("coordinates", "lonlat")).write(mesh);
        }

        if (!meshGeneratorParams.fileXY_.empty()) {
            atlas::output::PathName path(meshGeneratorParams.fileXY_);
            log << "Mesh: writing to '" << path << "'" << std::endl;
            atlas::output::Gmsh(path, atlas::util::Config("coordinates", "xy")).write(mesh);
        }

        if (!meshGeneratorParams.fileXYZ_.empty()) {
            atlas::output::PathName path(meshGeneratorParams.fileXYZ_);
            log << "Mesh: writing to '" << path << "'" << std::endl;
            atlas::output::Gmsh(path, atlas::util::Config("coordinates", "xyz")).write(mesh);
        }
    }
    catch (...) {
        // Make sure we don't leave an incomplete entry in the cache
        mesh_cache.erase(md5);
        throw;
    }

    mesh_cache.footprint(md5, caching::InMemoryCacheUsage(mesh.footprint(), 0));

    ASSERT(mesh.generated());
    return mesh;
}


FiniteElement::~FiniteElement() = default;


void FiniteElement::print(std::ostream& out) const {
    out << "FiniteElement[method=" << name() << ",";
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
    eckit::Channel& log = eckit::Log::debug<LibMir>();

    log << "FiniteElement::assemble (input: " << in << ", output: " << out << ")" << std::endl;


    // get input mesh
    ASSERT(meshGeneratorParams().meshCellCentres_);  // required for the k-d tree

    const atlas::Mesh& inMesh    = atlasMesh(statistics, in);
    const util::Domain& inDomain = in.domain();

    const atlas::mesh::Nodes& inNodes          = inMesh.nodes();
    atlas::array::ArrayView<double, 2> icoords = atlas::array::make_view<double, 2>(inNodes.field("xyz"));

    size_t firstVirtualPoint = std::numeric_limits<size_t>::max();
    if (inNodes.metadata().has("NbRealPts")) {
        firstVirtualPoint = inNodes.metadata().get<size_t>("NbRealPts");
    }


    // generate k-d tree with cell centres
    std::unique_ptr<element_tree_t> eTree;
    {
        eckit::ResourceUsage usage("FiniteElement::assemble create k-d tree", log);
        eckit::TraceTimer<LibMir> timer("k-d tree: create");
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
        Pretty::ProgressTimer progress("Projecting", nbOutputPoints, {"point"}, log);

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
                triplet_vector_t triplets   = projectPointTo3DElements(nbInputPoints, icoords, connectivity, p, ip,
                                                                     firstVirtualPoint, nbProjectionAttempts, closest);

                nbMaxElementsSearched   = std::max(nbMaxElementsSearched, closest.size());
                nbMinElementsSearched   = std::min(nbMinElementsSearched, closest.size());
                nbMaxProjectionAttempts = std::max(nbMaxProjectionAttempts, nbProjectionAttempts);

                if (triplets.empty()) {
                    // If this fails, consider lowering parametricEpsilon
                    failures.push_front(failed_projection_t(ip, it->pointUnrotated()));
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

    log << "Projected " << Pretty(nbProjections) << " of " << Pretty(nbOutputPoints, {"point"}) << " ("
        << Pretty(nbFailures, {"failure"}) << ")\n"
        << "k-d tree: searched between " << Pretty(nbMinElementsSearched) << " and "
        << Pretty(nbMaxElementsSearched, {"element"}) << ", with up to "
        << Pretty(nbMaxProjectionAttempts, {"projection attempt"}) << " (per point)" << std::endl;

    if ((nbFailures > 0) && !failuresAreMissingValues) {
        std::stringstream msg;
        msg << "Failed to project " << Pretty(nbFailures, {"point"});
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


FiniteElementFactory::FiniteElementFactory(const std::string& name) : MethodFactory(name), name_(name) {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    if (m->find(name) != m->end()) {
        throw eckit::SeriousBug("FiniteElementFactory: duplicate '" + name + "'");
    }

    ASSERT(m->find(name) == m->end());
    (*m)[name] = this;
}


FiniteElementFactory::~FiniteElementFactory() {
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    m->erase(name_);
}


void FiniteElementFactory::list(std::ostream& out) {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    const char* sep = "";
    for (const auto& j : *m) {
        out << sep << j.first;
        sep = ", ";
    }
}


FiniteElement* FiniteElementFactory::build(const std::string& method, const std::string& label,
                                           const param::MIRParametrisation& param) {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    eckit::Log::debug<LibMir>() << "FiniteElementFactory: looking for '" << method << "'" << std::endl;

    auto j = m->find(method);
    if (j == m->end()) {
        list(eckit::Log::error() << "FiniteElementFactory: unknown '" << method << "', choices are: ");
        throw eckit::SeriousBug("FiniteElementFactory: unknown '" + method + "'");
    }

    return j->second->make(param, label);
}


}  // namespace fe
}  // namespace method
}  // namespace mir
