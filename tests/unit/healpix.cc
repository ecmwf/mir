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
#include "mir/input/GribFileInput.h"
#include "mir/output/ArrayOutput.h"


namespace mir::tests::unit {


CASE("HEALPix") {
    SECTION("respect order") {
        api::MIRJob job;
        job.set("grid", "H16");
        job.set("order", "nested");

        output::ArrayOutput output;
        std::unique_ptr<input::MIRInput> input(
            new input::GribFileInput("gridType=healpix,Nside=2,orderingConvention=nested.grib2"));
        ASSERT(input->next());

        job.execute(*input, output);

        EXPECT(output.gridspec() == R"({"grid":"H16","order":"nested"})");
    }
}


}  // namespace mir::tests::unit


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
