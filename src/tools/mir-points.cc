/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Pedro Maciel
/// @date Apr 2015


#include "eckit/option/CmdArgs.h"
#include "eckit/types/Types.h"
#include "mir/action/context/Context.h"
#include "mir/api/MIRJob.h"
#include "mir/input/GribFileInput.h"
#include "mir/output/ValuesOutput.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/tools/MIRTool.h"


class MIRPoints : public mir::tools::MIRTool {

    // -- Overridden methods

    void execute(const eckit::option::CmdArgs&);

    void usage(const std::string &tool) const;

public:

    // -- Contructors

    MIRPoints(int argc, char **argv) : mir::tools::MIRTool(argc, argv) {}

};


void MIRPoints::usage(const std::string &tool) const {
    eckit::Log::info()
            << "\n" "Usage: " << tool
            << std::endl;
}


void MIRPoints::execute(const eckit::option::CmdArgs& args) {

    mir::input::GribFileInput input(args(0));
    mir::output::ValuesOutput output;

    mir::api::MIRJob job;

    std::vector<double> latitudes = {50, 30, 20, 10, 0};
    std::vector<double> longitudes = { -10, 10, 6, 52, 8};

    job.set("caching", false);

    job.set("latitudes", latitudes);
    job.set("longitudes", longitudes);

    job.set("interpolation", "k-nearest");
    job.set("number_of_closest_points", 1);
    job.set("lsm", false);

    while (input.next()) {
        job.execute(input, output);

        std::cout << "Number of fields: " << output.dimensions() << std::endl;
        std::cout << "Values " << output.values() << std::endl;
        std::cout << "Has missing " << output.hasMissing() << std::endl;
        std::cout << "Missing " << output.missingValue() << std::endl;

    }
}


int main(int argc, char **argv) {
    MIRPoints tool(argc, argv);
    return tool.start();
}


