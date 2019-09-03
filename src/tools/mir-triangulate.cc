/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include <iomanip>
#include <sstream>

#include "eckit/log/Log.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/option/SimpleOption.h"

#include "atlas/output/Gmsh.h"
#include "atlas/util/CoordinateEnums.h"


#include "mir/api/Atlas.h"
#include "mir/data/MIRField.h"
#include "mir/input/GribFileInput.h"
#include "mir/repres/Representation.h"
#include "mir/tools/MIRTool.h"


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


    // loop over each file(s) message(s)
    for (size_t i = 0; i < args.count(); ++i) {
        mir::input::GribFileInput grib(args(i));
        while (grib.next()) {

            // Build a mesh from grid
            mir::repres::RepresentationHandle rep(
                static_cast<const mir::input::MIRInput&>(grib).field().representation());


            MeshGenerator generate("delaunay");
            Mesh mesh = generate(rep->atlasGrid());

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
            auto& connectivity = mesh.cells().node_connectivity();
            auto coord         = array::make_view<double, 2, atlas::array::Intent::ReadOnly>(mesh.nodes().lonlat());

            log << "---" "\nfile: " << args(i) << "\nnumberOfElements: " << connectivity.rows() << "\nelements:";

            for (idx_t e = 0; e < connectivity.rows(); ++e) {
                auto row = connectivity.row(e);
                ASSERT(row.size() == 3);
                log << "\n  - "
                    << coord(row(0), LAT) << " " << coord(row(0), LON) << " "
                    << coord(row(1), LAT) << " " << coord(row(1), LON) << " "
                    << coord(row(2), LAT) << " " << coord(row(2), LON);
            }

            log << std::endl;

        }
    }
}


int main(int argc, char** argv) {
    MIRTriangulate tool(argc, argv);
    return tool.start();
}
