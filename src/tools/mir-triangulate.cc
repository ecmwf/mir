/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include <algorithm>
#include <cmath>
#include <iomanip>
#include <limits>
#include <memory>
#include <sstream>
#include <vector>

#include "eckit/log/Log.h"
#include "eckit/log/Timer.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/option/SimpleOption.h"

#include "atlas/output/Gmsh.h"
#include "atlas/util/CoordinateEnums.h"

#include "mir/api/Atlas.h"
#include "mir/data/MIRField.h"
#include "mir/input/GribFileInput.h"
#include "mir/repres/Iterator.h"
#include "mir/repres/Representation.h"
#include "mir/tools/MIRTool.h"

#include "mir/util/MeshGeneratorParameters.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/param/CombinedParametrisation.h"


class MIRTriangulate : public mir::tools::MIRTool {
private:
    void execute(const eckit::option::CmdArgs&);
    void usage(const std::string& tool) const;
    int minimumPositionalArguments() const {
        return 0;
    }
public:
    MIRTriangulate(int argc, char **argv) : mir::tools::MIRTool(argc, argv) {
        options_.push_back(new eckit::option::SimpleOption<std::string>("output", "Gmsh output file"));
    }
};


void MIRTriangulate::usage(const std::string &tool) const {
    eckit::Log::info() <<
                       "\nGRIB Delaunay triangulation."
                       "\n"
                       "\nUsage:"
                       "\n\t" << tool << " [--output=example.msh] file.grib [file.grib [...]]"
                       << std::endl;
}


void MIRTriangulate::execute(const eckit::option::CmdArgs& args) {
    using namespace atlas;
    auto& log = eckit::Log::info();

    std::string output = args.getString("output", "");
    bool alternate     = args.getBool("alternate", false);


    // loop over each file(s) message(s)
    for (size_t i = 0; i < args.count(); ++i) {

        mir::input::GribFileInput grib(args(i));
        const mir::input::MIRInput& input = grib;

        while (grib.next()) {
            eckit::Timer tim(alternate ? "Delaunay triangulation (alternate)" : "Delaunay triangulation");

            auto field = input.field();

            // Build a mesh from grid
            mir::repres::RepresentationHandle rep(field.representation());


            mir::param::SimpleParametrisation simple;
            mir::param::CombinedParametrisation ignore(simple, simple, simple);

            mir::util::MeshGeneratorParameters param("input", ignore);
            rep->fill(param);

            MeshGenerator generate(param.meshGenerator_, param);
            Mesh mesh = generate(rep->atlasGrid());

            const auto& values = field.values(0);

            if (!output.empty()) {
                eckit::PathName path(output);

                for (size_t counter = 1; path.exists(); ++counter) {
                    std::ostringstream name;
                    name << output << "." << std::setw(4) << std::setfill('0') << counter;
                    path = name.str();
                }

                output::Gmsh(path).write(mesh);
            }


            // Write mesh connectivity
            const auto& connectivity = mesh.cells().node_connectivity();
            const auto coord         = array::make_view<double, 2, atlas::array::Intent::ReadOnly>(mesh.nodes().lonlat());

            log << "---" "\nfile: " << args(i) << "\nnumberOfElements: " << connectivity.rows() << "\nelements:";

            for (idx_t e = 0; e < connectivity.rows(); ++e) {
                const auto row = connectivity.row(e);

                switch (row.size()) {

                case 3:
                    log << "\n  - "
                        << coord(row(0), LAT) << " " << coord(row(0), LON) << " "
                        << coord(row(1), LAT) << " " << coord(row(1), LON) << " "
                        << coord(row(2), LAT) << " " << coord(row(2), LON) << " ---> "
                        << values[row(0)];
                    break;

                case 4:
                    log << "\n  - "
                        << coord(row(0), LAT) << " " << coord(row(0), LON) << " "
                        << coord(row(1), LAT) << " " << coord(row(1), LON) << " "
                        << coord(row(2), LAT) << " " << coord(row(2), LON) << " "
                        << coord(row(3), LAT) << " " << coord(row(3), LON)<< " ---> "
                        << values[row(0)];

                        ;
                    break;
                default:
                    NOTIMP;
                    break;
                }
                // ASSERT(row.size() == 3);

            }

            log << std::endl;

        }
    }
}


int main(int argc, char** argv) {
    MIRTriangulate tool(argc, argv);
    return tool.start();
}
