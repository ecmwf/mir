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
#include <vector>

#include "eckit/testing/Test.h"

#include "mir/api/MIRJob.h"
#include "mir/input/GribFileInput.h"
#include "mir/input/RawInput.h"
#include "mir/output/ArrayOutput.h"
#include "mir/param/GridSpecParametrisation.h"


namespace mir::tests::unit {


CASE("HEALPix") {
    const std::string gridspec_ring   = R"({"grid":"H4"})";
    const std::string gridspec_nested = R"({"grid":"H4","order":"nested"})";

    struct test_type {
        std::string grid;
        const std::string& gridspec;
    };

    const std::vector<test_type> tests{
        {"H4", gridspec_ring},
        {"hR4", gridspec_ring},
        {"H4r", gridspec_ring},
        {"{grid: H4, order: ring}", gridspec_ring},
        {"h4n", gridspec_nested},
        {"HN4", gridspec_nested},
        {"{grid: H4, order: nested}", gridspec_nested},
    };


    SECTION("GRIB-based input") {
        for (const std::string& in : {
                 "gridType=healpix,Nside=2,orderingConvention=ring.grib2",
                 "gridType=healpix,Nside=2,orderingConvention=nested.grib2",
             }) {
            for (const auto& out : tests) {
                api::MIRJob job;
                job.set("grid", out.grid);
                job.set("interpolation", "nn");

                std::unique_ptr<input::MIRInput> input(new input::GribFileInput(in));
                ASSERT(input->next());

                output::ArrayOutput result;
                job.execute(*input, result);

                EXPECT(result.gridspec() == out.gridspec);
            }
        }
    }


    SECTION("memory-based input") {
        for (const auto& in : tests) {
            param::GridSpecParametrisation param(in.grid);
            std::vector<double> values(param.grid().size(), 0.);

            EXPECT(param.spec().str() == in.gridspec);

            for (const auto& out : tests) {
                api::MIRJob job;
                job.set("grid", out.grid);
                job.set("interpolation", "nn");

                std::unique_ptr<input::MIRInput> input(new input::RawInput(values.data(), values.size(), param));
                ASSERT(input->next());

                output::ArrayOutput result;
                job.execute(*input, result);

                EXPECT(result.gridspec() == out.gridspec);
            }
        }
    }
}


}  // namespace mir::tests::unit


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
