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


#include <memory>

#include "eckit/testing/Test.h"

#include "mir/api/MIRJob.h"
#include "mir/api/mir_config.h"
#include "mir/input/MIRInput.h"
#include "mir/output/EmptyOutput.h"
#include "mir/param/SimpleParametrisation.h"


namespace mir::tests::unit {


CASE("interpolations") {
    api::MIRJob jobs[3];  // jobs[0]: no post-processing

    jobs[0].set("caching", false);
    jobs[0].set("grid", "3/3");

    jobs[1].set("caching", false);
    jobs[1].set("grid", "1/1");
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
