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
#include <sstream>
#include <string>
#include <vector>

#include "eckit/filesystem/PathName.h"
#include "eckit/parser/YAMLParser.h"
#include "eckit/testing/Test.h"

#include "mir/api/MIRJob.h"
#include "mir/input/RawInput.h"
#include "mir/output/EmptyOutput.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/util/Log.h"


namespace mir::tests::unit {


CASE("InterpolationSpec integration test") {
    auto& log = Log::info();


    // temporary file for dumping weights info_path (should be local and unique)
    eckit::PathName info_path{
        "mir_tests_unit"
        "_"
        "earthkit-regrid"
        "_"
        "dump-weights-info"};


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

    std::vector<double> values(9, 0.);

    // extract "interpolation" node from the dumped info
    auto info_interpolation = [](const auto& path) {
        auto value = eckit::YAMLParser::decodeFile(path);
        std::ostringstream ss;
        ss << value["interpolation"];
        return ss.str();
    };


    static const std::string EXPECTED_INTERPOLATION_LINEAR{"{engine => mir , version => 16 , method => linear}"};
    static const std::string EXPECTED_INTERPOLATION_NN{"{engine => mir , version => 16 , method => nearest-neighbour}"};
    static const std::string EXPECTED_INTERPOLATION_GBA{"{engine => mir , version => 16 , method => grid-box-average}"};


    SECTION("interpolation=linear (default)") {
        std::unique_ptr<input::MIRInput> input(new input::RawInput(values.data(), values.size(), meta));
        std::unique_ptr<output::MIROutput> output(new output::EmptyOutput);

        api::MIRJob job;
        job.set("grid", std::vector<double>{2., 2.});
        job.set("caching", false);
        job.set("interpolation", "linear");
        job.set("dump-weights-info", info_path);
        job.execute(*input, *output);

        EXPECT(info_interpolation(info_path) == EXPECTED_INTERPOLATION_LINEAR);
    }


    SECTION("interpolation=linear with non-default option(s)") {
        std::unique_ptr<input::MIRInput> input(new input::RawInput(values.data(), values.size(), meta));
        std::unique_ptr<output::MIROutput> output(new output::EmptyOutput);

        api::MIRJob job;
        job.set("grid", std::vector<double>{2., 2.});
        job.set("caching", false);
        job.set("interpolation", "linear");
        job.set("dump-weights-info", info_path);
        job.set("finite-element-validate-mesh", true);  // non-default option
        job.execute(*input, *output);

        EXPECT(info_interpolation(info_path) != EXPECTED_INTERPOLATION_LINEAR);
    }


    SECTION("interpolation=nn (default)") {
        std::unique_ptr<input::MIRInput> input(new input::RawInput(values.data(), values.size(), meta));
        std::unique_ptr<output::MIROutput> output(new output::EmptyOutput);

        api::MIRJob job;
        job.set("grid", std::vector<double>{2., 2.});
        job.set("caching", false);
        job.set("interpolation", "nn");
        job.set("dump-weights-info", info_path);
        job.execute(*input, *output);

        EXPECT(info_interpolation(info_path) == EXPECTED_INTERPOLATION_NN);
    }


    SECTION("interpolation=nn with non-default option(s)") {
        std::unique_ptr<input::MIRInput> input(new input::RawInput(values.data(), values.size(), meta));
        std::unique_ptr<output::MIROutput> output(new output::EmptyOutput);

        api::MIRJob job;
        job.set("grid", std::vector<double>{2., 2.});
        job.set("caching", false);
        job.set("interpolation", "nn");
        job.set("dump-weights-info", info_path);
        job.set("nclosest", 2);  // non-default option
        job.execute(*input, *output);

        EXPECT(info_interpolation(info_path) != EXPECTED_INTERPOLATION_NN);
    }


    SECTION("interpolation=grid-box-average (default)") {
        std::unique_ptr<input::MIRInput> input(new input::RawInput(values.data(), values.size(), meta));
        std::unique_ptr<output::MIROutput> output(new output::EmptyOutput);

        api::MIRJob job;
        job.set("grid", std::vector<double>{2., 2.});
        job.set("caching", false);
        job.set("interpolation", "grid-box-average");
        job.set("dump-weights-info", info_path);
        job.execute(*input, *output);
        EXPECT(info_interpolation(info_path) == EXPECTED_INTERPOLATION_GBA);
    }


    SECTION("interpolation=grid-box-average with non-default option(s)") {
        std::unique_ptr<input::MIRInput> input(new input::RawInput(values.data(), values.size(), meta));
        std::unique_ptr<output::MIROutput> output(new output::EmptyOutput);

        api::MIRJob job;
        job.set("grid", std::vector<double>{2., 2.});
        job.set("caching", false);
        job.set("interpolation", "grid-box-average");
        job.set("dump-weights-info", info_path);
        job.set("prune-epsilon", 1e-2);  // non-default option
        job.execute(*input, *output);

        EXPECT(info_interpolation(info_path) != EXPECTED_INTERPOLATION_GBA);
    }
}


}  // namespace mir::tests::unit


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
