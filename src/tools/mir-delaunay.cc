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


extern "C" {
#include "libqhull_r/qhull_ra.h"
}

#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

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

            auto nbPoints = repres->numberOfPoints();
            auto points   = new coordT[3 * nbPoints];  //  Qhull-handled

            {
                auto pp = &points[0];
                for (std::unique_ptr<repres::Iterator> iterator(repres->iterator()); iterator->next();) {
                    auto p = iterator->point3D();
                    *pp++  = p[XYZCOORDS::XX];
                    *pp++  = p[XYZCOORDS::YY];
                    *pp++  = p[XYZCOORDS::ZZ];
                }
            }

            // Qhull's data structure
            qhT qh;

            QHULL_LIB_CHECK

            qh_zero(&qh, stderr);

            auto flags = const_cast<char*>("qhull Qt");
            // For a Voronoi diagram ('v') call qh_setvoronoi_all() after qh_new_qhull()

            ASSERT(0 == qh_new_qhull(&qh, 3, int(nbPoints), points, true, flags, stdout, stderr));


            std::ofstream f("out.msh");
            f << "$MeshFormat"
                 "\n2.0 0 8"
                 "\n$EndMeshFormat";

            {
                f << "\n$Nodes"
                     "\n"
                  << nbPoints;

#if 1
                auto pp = &points[0];
                for (size_t i = 1; i <= nbPoints; ++i) {
                    auto x = pp++;
                    auto y = pp++;
                    auto z = pp++;
                    f << '\n' << i << ' ' << *x << ' ' << *y << ' ' << *z;
                }
#else
                for (auto vx = qh.vertex_list; (vx != nullptr) && (vx->next != nullptr); vx = vx->next) {
                    auto p = vx->point;
                    f << '\n' << i++ << ' ' << p[XYZCOORDS::XX] << ' ' << p[XYZCOORDS::YY] << ' ' << p[XYZCOORDS::ZZ];
                }
#endif
                f << "\n$EndNodes";
            }

            {
                f << "\n$Elements"
                     "\n"
                  << qh.num_facets;

                size_t i = 1;
                for (auto facet = qh.facet_list; (facet != nullptr) && (facet->next != nullptr); facet = facet->next) {
                    std::vector<unsigned int> vertex;
                    vertex.reserve(3);

                    for (vertexT *v, **vp = reinterpret_cast<vertexT**>(&(facet->vertices->e[0].p));
                         (v = *vp++) != nullptr && vertex.size() <= 3;) {
                        vertex.emplace_back(v->id);
                    }

                    ASSERT(vertex.size() == 3);
                    f << "\n" << i++ << " 2 1 99 " << vertex[0] << ' ' << vertex[1] << ' ' << vertex[2];
                }
                f << "\n$EndElements";
            }

            f << "\n";


            Log::info() << qh.num_vertices << " vertices and " << qh.num_facets << " facets" << std::endl;
            //            for (auto facet = qh.facet_list; (facet != nullptr) && (facet->next != nullptr); facet =
            //            facet->next) {

            //                Log::info() << "x" << std::endl;
            //                for (vertexT *vertex = nullptr, **vertexp =
            //                reinterpret_cast<vertexT**>(&(facet->vertices->e[0].p));
            //                     (vertex = *vertexp++);) {
            //                    Log::info() << "v: " << vertex->point << " " << vertex->id << std::endl;
            //                }
            //            }


            qh.NOerrexit = True;

            qh_freeqhull(&qh, qh_ALL);
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
