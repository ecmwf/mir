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
#include <string>

#include "eckit/testing/Test.h"

#include "mir/api/MIRJob.h"
#include "mir/api/mir_config.h"
#include "mir/input/MIRInput.h"
#include "mir/output/EmptyOutput.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/util/Atlas.h"


namespace mir::tests::unit {


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

        for (const auto* in : {
                 "../data/param=2t,levtype=sfc,grid=O640",
                 "../data/regular_ll.2-4.grib1",
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
            std::unique_ptr<input::MIRInput> input(
                input::MIRInputFactory::build("../data/param=t,level=1000,resol=20", args));
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
                input::MIRInputFactory::build("../data/param=vo_d,level=1000,resol=20", args));
            output::EmptyOutput output;

            job.set("vod2uv", true);

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
