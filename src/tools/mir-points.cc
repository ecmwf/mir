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


#include "eckit/option/CmdArgs.h"

#include "mir/api/MIRJob.h"
#include "mir/input/GribFileInput.h"
#include "mir/output/ValuesOutput.h"
#include "mir/tools/MIRTool.h"
#include "mir/util/Log.h"


using namespace mir;


struct MIRPoints : tools::MIRTool {
    using MIRTool::MIRTool;

    void usage(const std::string& tool) const override {
        Log::info() << "\n"
                       "Usage: "
                    << tool << std::endl;
    }

    void execute(const eckit::option::CmdArgs&) override;
};


void MIRPoints::execute(const eckit::option::CmdArgs& args) {
    input::GribFileInput input(args(0));
    output::ValuesOutput output;

    api::MIRJob job;

    std::vector<double> latitudes  = {50, 30, 20, 10, 0};
    std::vector<double> longitudes = {-10, 10, 6, 52, 8};

    job.set("caching", false);

    job.set("latitudes", latitudes);
    job.set("longitudes", longitudes);

    job.set("interpolation", "k-nearest");
    job.set("number_of_closest_points", 1);
    job.set("lsm", false);

    while (input.next()) {
        job.execute(input, output);

        Log::info() << "Number of fields: " << output.dimensions() << std::endl;
        Log::info() << "Values " << output.values() << std::endl;
        Log::info() << "Has missing " << output.hasMissing() << std::endl;
        Log::info() << "Missing " << output.missingValue() << std::endl;
    }
}


int main(int argc, char** argv) {
    MIRPoints tool(argc, argv);
    return tool.start();
}
