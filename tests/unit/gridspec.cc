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
#include "mir/input/MIRInput.h"
#include "mir/input/RawInput.h"
#include "mir/output/EmptyOutput.h"
#include "mir/param/SimpleParametrisation.h"


namespace mir::tests::unit {


CASE("gridspec") {
    SECTION("output {grid: 1/1}") {
        std::vector<double> values(9, 0.);
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

        output::EmptyOutput output;

        api::MIRJob job;
        job.set("grid", "{grid: 1/1}");

        const param::SimpleParametrisation args;
        for (std::unique_ptr<input::MIRInput> input(new input::RawInput(values.data(), values.size(), meta));
             input->next();) {
            job.execute(*input, output);
        }
    }
}


}  // namespace mir::tests::unit


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
