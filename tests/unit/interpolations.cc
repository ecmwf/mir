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
    api::MIRJob jobs[7];  // jobs[0]: no post-processing

    jobs[1].set("caching", false);
    jobs[1].set("grid", "2/2");

    jobs[2].set("caching", false);
    jobs[2].set("grid", "3/3");

    jobs[3].set("caching", false);
    jobs[3].set("grid", "1/1");
    jobs[3].set("area", "40/20/20/40");

    jobs[4].set("caching", false);
    jobs[4].set("grid", "1/1");
    jobs[4].set("area", "40/20/20/40");
    jobs[4].set("frame", 2);

#if mir_HAVE_ATLAS
    jobs[5].set("caching", false);
    jobs[5].set("rotation", "-90/0");

    jobs[6].set("caching", false);
    jobs[6].set("rotation", "-89/10");
#endif


    SECTION("gridded to gridded (GRIB)") {
        param::SimpleParametrisation args;

        for (const std::string& in : {
                 "../data/param=2t,levtype=sfc,grid=F640",
                 "../data/param=2t,levtype=sfc,grid=N640",
                 "../data/param=2t,levtype=sfc,grid=O640",
                 "../data/regular_ll.2-2.grib2",
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


#if mir_HAVE_NETCDF
#if atlas_HAVE_TESSELATION
    SECTION("gridded to gridded (netCDF)") {
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
#endif
#endif


#if mir_HAVE_ATLAS
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
#endif
}


#if mir_HAVE_ATLAS
CASE("MIR-583") {
    // interpolation=linear failure on low parametricEpsilon

    api::MIRJob job;
    job.set("caching", false);
    job.set("grid", "0.04/0.04");
    job.set("interpolation", "linear");
    job.set("finite-element-missing-value-on-projection-fail", false);

    param::SimpleParametrisation args;
    std::unique_ptr<input::MIRInput> input(input::MIRInputFactory::build("MIR-583.grib1", args));

    output::EmptyOutput output;

    while (input->next()) {
        job.execute(*input, output);
    }
}
#endif


}  // namespace mir::tests::unit


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
