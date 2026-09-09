// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "eckit/option/CmdArgs.h"

#include "mir/api/MIRJob.h"
#include "mir/input/GribFileInput.h"
#include "mir/output/ValuesOutput.h"
#include "mir/tools/MIRTool.h"
#include "mir/util/Log.h"


namespace mir::tools {


struct MIRPoints : MIRTool {
    using MIRTool::MIRTool;

    void usage(const std::string& tool) const override {
        Log::info() << "\n"
                       "Usage: "
                    << tool << std::endl;
    }

    void execute(const eckit::option::CmdArgs& args) override;
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


}  // namespace mir::tools


int main(int argc, char** argv) {
    mir::tools::MIRPoints tool(argc, argv);
    return tool.start();
}
