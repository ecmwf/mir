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

            std::vector<Point3> points;
            points.reserve(nbPoints);

            for (std::unique_ptr<repres::Iterator> iterator(repres->iterator()); iterator->next();) {
                points.emplace_back(iterator->point3D());
            }

            // Qhull's data structure
            qhT qh_qh;
            qhT* qh = &qh_qh;

            QHULL_LIB_CHECK

            qh_zero(qh, stderr);

            const char* flags_convexhull = "qhull s Tcv";
            const char* flags_delaunay   = "qhull s d Tcv";
            // For a Voronoi diagram ('v') call qh_setvoronoi_all() after qh_new_qhull()

            ASSERT(0 == qh_new_qhull(qh, 3, int(nbPoints), reinterpret_cast<coordT*>(points.data()), true,
                                     const_cast<char*>(flags_convexhull), nullptr, stderr));

            Log::info() << qh->num_vertices << " vertices and " << qh->num_facets << " facets" << std::endl;
            for (auto facet = qh->facet_list; (facet != nullptr) && (facet->next != nullptr); facet = facet->next) {
            }


            qh->NOerrexit = True;

            qh_freeqhull(qh, qh_ALL);
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
