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
#include <numeric>
#include <ostream>
#include <vector>

#include "eckit/testing/Test.h"

#include "mir/api/MIRJob.h"
#include "mir/data/MIRField.h"
#include "mir/input/RawInput.h"
#include "mir/output/RawOutput.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/util/Log.h"


namespace mir {
namespace tests {
namespace unit {


CASE("RawInput") {
    auto& log = Log::info();


    SECTION("grid=1/1") {
        // metadata
        param::SimpleParametrisation meta;

        meta.set("gridded", true);
        meta.set("gridType", "regular_ll");
        meta.set("north", 10.);
        meta.set("west", 0.);
        meta.set("south", 0.);
        meta.set("east", 10.);
        meta.set("south_north_increment", 5.);
        meta.set("west_east_increment", 5.);
        meta.set("Ni", 3);
        meta.set("Nj", 3);


        // data
        std::vector<double> values(9, 0.);
        std::unique_ptr<input::MIRInput> input(new input::RawInput(values.data(), values.size(), meta));


        // access a field (in the post-processing context)
        log << *input << std::endl;

        auto field = input->field();
        log << field << std::endl;
    }


    SECTION("grid=o4") {
        // metadata
        param::SimpleParametrisation meta;

        meta.set("gridded", true);
        meta.set("gridType", "reduced_gg");
        meta.set("north", 90.);
        meta.set("west", 0.);
        meta.set("south", -90.);
        meta.set("east", 360.);
        meta.set("N", 4);
        meta.set("pl", std::vector<long>{20, 24, 28, 32, 32, 28, 24, 20});


        // data
        std::vector<double> values(208 /*sum(pl)*/, 0.);
        std::unique_ptr<input::MIRInput> input(new input::RawInput(values.data(), values.size(), meta));


        // access a field (in the post-processing context)
        log << *input << std::endl;

        auto field = input->field();
        log << field << std::endl;
    }
}


CASE("Example") {
    auto& log = Log::info();


    // input, with a values vector representing:
    // - North Pole
    // - 20 x 0.
    // - 24 x 0.
    // - 28 x 0.
    // - 32 x 42.
    // - Equator
    // - 32 x -42.
    // - 28 x 0.
    // - 24 x 0.
    // - 20 x 0.
    // - South Pole

    param::SimpleParametrisation meta;
    meta.set("gridded", true);
    meta.set("gridType", "reduced_gg");
    meta.set("north", 90.);
    meta.set("west", 0.);
    meta.set("south", -90.);
    meta.set("east", 360.);
    meta.set("N", 4);
    meta.set("pl", std::vector<long>{20, 24, 28, 32, 32, 28, 24, 20});

    std::vector<double> values(208 /*sum(pl)*/, 0.);
    std::fill_n(values.begin() + 20 + 24 + 28, 32, 42.);
    std::fill_n(values.begin() + 20 + 24 + 28 + 32, 32, -42.);

    std::unique_ptr<input::MIRInput> input(new input::RawInput(values.data(), values.size(), meta));


    // output
    std::vector<double> out(4, 0);
    std::unique_ptr<output::MIROutput> output(new output::RawOutput(out.data(), out.size()));


    // job
    api::MIRJob job;
    job.set("grid", std::vector<double>{2., 2.});
    job.set("area", std::vector<double>{1., -1., -1., 1.});
    job.set("interpolation", "nn");
    job.set("caching", false);

    log << job << std::endl;
    job.execute(*input, *output);

    EXPECT_EQUAL(out[0], 42.);
    EXPECT_EQUAL(out[1], 42.);
    EXPECT_EQUAL(out[2], -42.);
    EXPECT_EQUAL(out[3], -42.);
}


}  // namespace unit
}  // namespace tests
}  // namespace mir


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
