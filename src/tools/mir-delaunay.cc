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


#include <fstream>
#include <memory>
#include <vector>

#include "libqhullcpp/Qhull.h"
#include "libqhullcpp/QhullFacetList.h"
#include "libqhullcpp/QhullFacetSet.h"
#include "libqhullcpp/QhullVertexSet.h"
#include "libqhullcpp/RboxPoints.h"

#include "eckit/option/CmdArgs.h"
#include "eckit/option/SimpleOption.h"
#include "eckit/option/VectorOption.h"

#include "mir/data/MIRField.h"
#include "mir/input/MIRInput.h"
#include "mir/param/ConfigurationWrapper.h"
#include "mir/repres/Iterator.h"
#include "mir/repres/Representation.h"
#include "mir/tools/MIRTool.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"


namespace mir {
namespace tools {


struct MIRDelaunay : tools::MIRTool {
    MIRDelaunay(int argc, char** argv) : MIRTool(argc, argv) {
        // options_.push_back(new eckit::option::VectorOption<double>("grid", "West-East & South-North increments", 2));
        // options_.push_back(new eckit::option::SimpleOption<std::string>("gridname", "grid name"));
        // options_.push_back(new eckit::option::VectorOption<double>("area", "North/West/South/East", 4));
    }

    void usage(const std::string& tool) const override { Log::info() << tool << " ?" << std::endl; }

    void execute(const eckit::option::CmdArgs&) override;
};


void MIRDelaunay::execute(const eckit::option::CmdArgs& args) {
    const param::ConfigurationWrapper args_wrap(args);

    for (size_t i = 0, j = 0; i < args.count(); ++i) {
        std::unique_ptr<input::MIRInput> input(input::MIRInputFactory::build(args(i), args_wrap));

        while (input->next()) {
            repres::RepresentationHandle repres(input->field().representation());
            ASSERT(repres);

            Log::info() << "#" << ++j << ": " << *repres << std::endl;

            using orgQhull::Qhull;
            using orgQhull::RboxPoints;

            enum
            {
                EG_CONVEX,
                EG_DELAUNAY,
                EG_VORONOI
            } mode = EG_CONVEX;

            if (mode == EG_CONVEX) {
                RboxPoints rbox;
                rbox.setDimension(3);
                for (std::unique_ptr<repres::Iterator> iterator(repres->iterator()); iterator->next();) {
                    auto p = iterator->point3D();
                    rbox.append(3, p.data());
                }

                Qhull q(rbox, "s o Qt");
                q.setOutputStream(&Log::info());
                q.setErrorStream(&Log::error());

                // q.outputQhull();

                if (true) {
                    std::ofstream f("out.msh");
                    f << "$MeshFormat"
                         "\n2.0 0 8"
                         "\n$EndMeshFormat";

                    {
                        f << "\n$Nodes"
                             "\n"
                          << rbox.count();

                        size_t i = 1;
                        for (auto& p : rbox) {
                            f << '\n'
                              << i++ << ' ' << p[XYZCOORDS::XX] << ' ' << p[XYZCOORDS::YY] << ' ' << p[XYZCOORDS::ZZ];
                        }
                        f << "\n$EndNodes";
                    }

                    size_t isTopOrient = 0;
                    {
                        f << "\n$Elements"
                             "\n"
                          << q.facetCount();

                        size_t i = 1;
                        for (const auto& ft : q.facetList()) {
                            ASSERT(ft.isGood());

                            auto x = [](const orgQhull::QhullVertex& v) { return v.point().id() + 1; };
                            auto v = ft.vertices();
                            ASSERT(v.count() == 3);

                            if (ft.isTopOrient()) {
                                isTopOrient++;
                                f << '\n' << i++ << " 2 1 99 " << x(v[0]) << ' ' << x(v[1]) << ' ' << x(v[2]);
                            }
                            else {
                                f << '\n' << i++ << " 2 1 99 " << x(v[1]) << ' ' << x(v[0]) << ' ' << x(v[2]);
                            }
                        }

                        f << "\n$EndElements";
                    }

                    f << "\n";
                }
            }

            if (mode == EG_DELAUNAY) {
                RboxPoints rbox("y c D2");
                Log::info() << rbox;

                Qhull q(rbox, "d o");
                q.setOutputStream(&Log::info());
                q.outputQhull();
                // q.outputQhull("o") produces the same output

                Qhull qhull;
                qhull.setOutputStream(&Log::info());

                // The Delaunay diagram is equivalent to the convex hull of a paraboloid, one dimension higher

                // Input sites as a vector of vectors
                std::vector<std::vector<double> > inputSites;
                for (auto& point : qhull.points()) {
                    inputSites.push_back(point.toStdVector());
                }

                // Delaunay regions as a vector of vectors
                std::vector<std::vector<int> > regions;
                for (auto& f : qhull.facetList()) {
                    std::vector<int> vertices;
                    if (!f.isUpperDelaunay()) {
                        if (!f.isTopOrient() && f.isSimplicial()) { /* orient the vertices like option 'o' */
                            auto vs = f.vertices();
                            vertices.push_back(vs[1].point().id());
                            vertices.push_back(vs[0].point().id());
                            for (int i = 2; i < (int)vs.size(); ++i) {
                                vertices.push_back(vs[i].point().id());
                            }
                        }
                        else { /* note: for non-simplicial facets, this code does not duplicate option 'o', see
                                  qh_facet3vertex and qh_printfacetNvertex_nonsimplicial */
                            for (auto vertex : f.vertices()) {
                                vertices.push_back(vertex.point().id());
                            }
                        }
                        regions.push_back(vertices);
                    }
                }
            }

            if (mode == EG_VORONOI) {
                RboxPoints rbox("y c D2");
                Log::info() << rbox;

                Qhull q(rbox, "v o");
                q.setOutputStream(&Log::info());
                q.outputQhull();
                // q.outputQhull("o") produces the same output
                bool isLower;
                int voronoiVertexCount;
                q.prepareVoronoi(&isLower, &voronoiVertexCount);

                Qhull qhull;
                qhull.setOutputStream(&Log::info());

                // Voronoi vertex at infinity is not included
                int numfacets    = qhull.facetCount();
                size_t numpoints = qhull.points().size();

                // Gather Voronoi vertices
                std::vector<std::vector<double> > voronoiVertices;
                for (auto& facet : qhull.facetList()) {
                    if (facet.visitId() && facet.visitId() < numfacets) {
                        voronoiVertices.push_back(facet.getCenter().toStdVector());
                    }
                }

                // Gather Voronoi regions (qh_printvoronoi calls qh_pointvertex via qh_markvoronoi)
                std::vector<std::vector<int> > voronoiRegions(numpoints);
                for (auto& vertex : qhull.vertexList()) {
                    size_t numinf = 0;
                    std::vector<int> voronoiRegion;
                    for (auto neighbor : vertex.neighborFacets()) {
                        if (neighbor.visitId() == 0) {
                            if (numinf) {
                                numinf = 1;
                                voronoiRegion.push_back(-1);  // -1 indicates the Voronoi vertex at infinity
                            }
                        }
                        else if (neighbor.visitId() < numfacets) {
                            voronoiRegion.push_back(neighbor.visitId() - 1);
                        }
                    }

                    if (voronoiRegion.size() > numinf) {
                        int siteId = vertex.point().id();
                        if (0 <= siteId && siteId < int(numpoints)) {  // otherwise would indicate qh.other_points
                            voronoiRegions[siteId] = voronoiRegion;
                        }
                    }
                }
            }
        }
    }

    Log::info() << std::endl;
}


}  // namespace tools
}  // namespace mir


int main(int argc, char** argv) {
    mir::tools::MIRDelaunay tool(argc, argv);
    return tool.start();
}
