// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <memory>
#include <string>
#include <vector>

#include "eckit/testing/Test.h"

#include "mir/api/MIRJob.h"
#include "mir/api/mir_config.h"
#include "mir/input/MIRInput.h"
#include "mir/output/EmptyOutput.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/util/Atlas.h"


namespace mir::interpolation::tests {


CASE("interpolations") {
    api::MIRJob jobs[3];  // jobs[0]: no post-processing

    jobs[0].set("caching", false);
    jobs[0].set("grid", "3/3");

    jobs[1].set("caching", false);
    jobs[1].set("grid", "1/1");
    jobs[1].set("area", "40/20/20/40");
    jobs[1].set("frame", 2);

    jobs[2].set("caching", false);
    jobs[2].set("rotation", "-89/10");


    SECTION("gridded to gridded (GRIB)") {
        param::SimpleParametrisation args;

        for (const auto& in : std::vector<std::string>{
                 "param=2t,levtype=sfc,grid=O16",
                 "regular_ll.2-4.grib1",
             }) {
            for (const auto& job : jobs) {
                std::unique_ptr<input::MIRInput> input(input::MIRInputFactory::build(in, args));
                output::EmptyOutput output;

                while (input->next()) {
                    job.execute(*input, output);
                }
            }
        }
    }


    SECTION("spectral to gridded (scalar)") {
        param::SimpleParametrisation args;

        for (const auto& job : jobs) {
            std::unique_ptr<input::MIRInput> input(input::MIRInputFactory::build("param=t,level=1000,resol=20", args));
            output::EmptyOutput output;

            while (input->next()) {
                job.execute(*input, output);
            }
        }
    }


    SECTION("spectral to gridded (vod2uv)") {
        param::SimpleParametrisation args;
        args.set("vod2uv", true);

        for (auto& job : jobs) {
            std::unique_ptr<input::MIRInput> input(
                input::MIRInputFactory::build("param=vo_d,level=1000,resol=20", args));
            output::EmptyOutput output;

            job.set("vod2uv", true);

            while (input->next()) {
                job.execute(*input, output);
            }
        }
    }
}


}  // namespace mir::interpolation::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
