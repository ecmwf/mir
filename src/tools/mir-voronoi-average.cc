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


#include <algorithm>
#include <memory>
#include <set>
#include <vector>

#include "eckit/option/CmdArgs.h"
#include "libqhullcpp/Qhull.h"
#include "libqhullcpp/QhullFacetList.h"
#include "libqhullcpp/QhullVertexSet.h"

#include "mir/data/MIRField.h"
#include "mir/input/GribFileInput.h"
#include "mir/repres/Iterator.h"
#include "mir/repres/Representation.h"
#include "mir/tools/MIRTool.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"
#include "mir/util/Trace.h"


namespace mir::tools {


class Voronoi {
public:
    using coord_t = std::vector<double>;

    Voronoi(const coord_t& coord) {
        constexpr size_t N = 3;

        trace::Timer trace("Voronoi");

        ASSERT(0 < N && coord.size() % N == 0);

        auto pointDimension = static_cast<int>(N);
        auto pointCount     = static_cast<int>(coord.size() / N);

        qh_ = new orgQhull::Qhull;
        ASSERT(qh_ != nullptr);

        std::ostringstream err;
        qh_->setErrorStream(&err);
        qh_->setOutputStream(&Log::info());
        qh_->enableOutputStream();

        try {
            qh_->runQhull("", pointDimension, pointCount, coord.data(), "v Qz");
            ASSERT(qh_->qhullStatus() == 0);
        }
        catch (...) {
            ASSERT(false);
        }
    }

    Voronoi(const Voronoi&) = delete;
    Voronoi(Voronoi&&)      = delete;

    ~Voronoi() { delete qh_; }

    Voronoi& operator=(const Voronoi&) = delete;
    Voronoi& operator=(Voronoi&&)      = delete;

    coord_t list_voronoi_vertices() const {
        coord_t vertices;
        vertices.reserve(3 * qh_->facetCount());

        for (auto f : qh_->facetList()) {
            /*if (!f.isUpperDelaunay())*/ {
                if (auto centre = f.voronoiVertex(); centre.size() == 3) {
                    vertices.insert(vertices.end(), centre.begin(), centre.end());
                }
            }
        }

        return vertices;
    }

    std::vector<std::vector<size_t>> list_voronoi_facets() const {
        std::vector<std::vector<size_t>> facets;

        auto facet = [](const auto& f) {
            decltype(facets)::value_type facet;
            for (const auto& v : f.vertices()) {
                facet.emplace_back(v.id());
            }
            return facet;
        };

        for (const auto& f : qh_->facetList()) {
            if (!f.isUpperDelaunay()) {
                facets.emplace_back(facet(f));
            }
        }

        return facets;
    }

private:
    orgQhull::Qhull* qh_;
};


#if 0
struct Voronoi {
    using coord_t = eckit::maths::Qhull::coord_t;

    explicit Voronoi(const coord_t& coord) {
        trace::Timer timer("Voronoi");
        qh_ = std::make_unique<eckit::maths::Qhull>(3, coord, "v Qz");
        ASSERT(qh_);
    }

    std::vector<size_t> list_vertices() const { return qh_->list_vertices(); }

    std::vector<std::vector<size_t>> list_facets() const {

        std::vector<std::vector<size_t>> facets;
        facets.reserve(qh_->facetCount());

        for (const auto& facet : qh_->facetList()) {
            const auto vertices = facet.vertices();

            std::vector<size_t> f;
            f.reserve(vertices.size());

            for (const auto& vertex : vertices) {
                f.emplace_back(vertex.point().id());
            }

            facets.emplace_back(std::move(f));
        }

        return facets;


        return qh_->list_facets(); }

#if 0
    void writeGmshFile(const std::vector<double>& points, const std::vector<eckit::maths::Qhull::point_t>& voronoiVertices, const std::vector<std::set<int>>& voronoiRegions, std::ostream&out) {
        out << "$MeshFormat"
               "\n2.2 0 8"
               "\n$EndMeshFormat"
               "\n";

        // Write points
        out << "$Nodes\n";
        out << points.size() / 3 + voronoiVertices.size() << "\n";
        int nodeId = 1;
        for (size_t i = 0; i < points.size(); i += 3) {
            out << nodeId++ << " " << points[i] << " " << points[i + 1] << " " << points[i + 2] << "\n";
        }
        for (const auto& v : voronoiVertices) {
            out << nodeId++ << " " << v[0] << " " << v[1] << " " << v[2] << "\n";
        }
        out << "$EndNodes\n";

        // Write elements
        out << "$Elements\n";
        int numElements = 0;
        for (const auto& region : voronoiRegions) {
            numElements += region.size();
        }
        out << numElements << "\n";

        int elementId = 1;
        int offset = points.size() / 3;
        for (size_t i = 0; i < voronoiRegions.size(); ++i) {
            for (int index : voronoiRegions[i]) {
                out << elementId++ << " 2 2 " << i + 1 << " " << i + 1 << " " << i + 1 << " " << index + offset + 1 << "\n";
            }
        }
        out << "$EndElements\n";
    }
#endif

private:
    std::unique_ptr<eckit::maths::Qhull> qh_;
};
#endif


struct MIRVoronoiAverage : MIRTool {
    using MIRTool::MIRTool;

    int numberOfPositionalArguments() const override { return 1; }

    void usage(const std::string& tool) const override {
        Log::info() << "\nVoronoi regions."
                       "\n"
                       "\nUsage:"
                       "\n\t"
                    << tool << " file.grib" << std::endl;
    }

    void execute(const eckit::option::CmdArgs& args) override {
        ASSERT(args.count() == 1);


        // Input
        std::unique_ptr<input::MIRInput> input(new input::GribFileInput(args(0)));
        ASSERT(input->next());


        // Get field representation
        auto field = input->field();
        repres::RepresentationHandle rep{field.representation()};


        // Get field points
        Voronoi::coord_t coord(3 * rep->numberOfPoints());

        size_t i = 0;
        for (std::unique_ptr<repres::Iterator> it{rep->iterator()}; it->next();) {
            const auto p  = it->point3D();
            coord.at(i++) = p[0];
            coord.at(i++) = p[1];
            coord.at(i++) = p[2];
        }


        // Calculate Voronoi regions
        Voronoi v(coord);

        for (const auto& v : v.list_voronoi_vertices()) {
            Log::info() << "Vertex: " << v << std::endl;
        }

        for (const auto& f : v.list_voronoi_facets()) {
            Log::info() << "Facet: ";
            for (const auto& v : f) {
                Log::info() << v << " ";
            }
            Log::info() << std::endl;
        }


        // Write Gmsh file
        std::ofstream out("filename.gmsh");
        ASSERT(!out);
    }
};


}  // namespace mir::tools


int main(int argc, char** argv) {
    mir::tools::MIRVoronoiAverage tool(argc, argv);
    return tool.start();
}
