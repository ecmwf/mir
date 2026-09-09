// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <memory>

#include "eckit/testing/Test.h"

#include "mir/api/MIRJob.h"
#include "mir/api/mir_config.h"
#include "mir/input/MIRInput.h"
#include "mir/output/EmptyOutput.h"
#include "mir/param/SimpleParametrisation.h"


namespace mir::tests::unit {


CASE("interpolations") {
    api::MIRJob jobs[3];  // jobs[2]: no post-processing

    jobs[0].set("caching", false);
    jobs[0].set("grid", "3/3");
    jobs[0].set("interpolation", "nn");

    jobs[1].set("caching", false);
    jobs[1].set("grid", "1/1");
    jobs[1].set("interpolation", "nn");
    jobs[1].set("area", "40/20/20/40");
    jobs[1].set("frame", 2);

    SECTION("interpolation gridded to gridded (netCDF)") {
        param::SimpleParametrisation args;
        args.set("input", "checkDuplicatePoints: False");

        for (const auto& job : jobs) {
            std::unique_ptr<input::MIRInput> input(input::MIRInputFactory::build("../data/nemo.nc", args));
            output::EmptyOutput output;

            while (input->next()) {
                job.execute(*input, output);
            }
        }
    }
}


}  // namespace mir::tests::unit


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
