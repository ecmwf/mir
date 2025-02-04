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
#include "mir/api/mir_config.h"
#include "mir/input/RawInput.h"
#include "mir/output/EmptyOutput.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/util/Log.h"


namespace mir::tests::unit {


CASE("InterpolationSpec") {
    // output
    std::unique_ptr<output::MIROutput> output(new output::EmptyOutput);

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


CASE("GridSpec") {
    auto& log = Log::info();

    // output
    std::unique_ptr<output::MIROutput> output(new output::EmptyOutput);


    SECTION("HEALPix grids") {
        for (const auto& tests : []() {
                 std::vector<std::string> tests;
                 for (std::string ordering : {"", "ring", "nested"}) {
                     for (size_t N : {2, 4, 8, 16, 32, 64, 128, 256, 512, 1024}) {
                         tests.emplace_back(ordering.empty()
                                                ? "{grid: H" + std::to_string(N)
                                                : "{grid: H" + std::to_string(N) + ", ordering: " + ordering + "}");
                     }
                 }
                 return tests;
             }()) {
        }
    }


    SECTION("Gaussian grids") {
        for (const auto& tests : []() {
                 std::vector<std::string> tests;
                 for (const std::string& type : {"F", "O", "N"}) {
                     for (size_t N : {2, 4, 8, 16, 32, 64, 128, 256, 512, 1024}) {
                         tests.emplace_back("{grid: " + type + std::to_string(N) + "}");
                     }
                     if (type == "O") {
                         tests.emplace_back("{grid: O2560}");
                     }
                 }
                 return tests;
             }()) {
            // TODO
        }
    }


    SECTION("Regular lat/lon grids") {
        for (const auto& tests : []() {
                 std::vector<std::string> tests;
                 for (double l : {
                          0.1, 0.125, 0.15, 0.2,  0.25, 0.3, 0.4, 0.5, 0.6, 0.7, 0.75, 0.8,
                          0.9, 1.,    1.2,  1.25, 1.4,  1.5, 1.6, 1.8, 10., 2.,  2.5,  5.,
                      }) {
                     tests.emplace_back("{grid: [" + std::to_string(l) + ", " + std::to_string(l) + "]}");
                 }
                 return tests;
             }()) {
            // TODO
        }
    }


    SECTION("ORCA grids") {
        for (const auto& test : std::vector<std::string>{
                 "eORCA025_T",
             }) {
            // TODO
        }
    }
}


}  // namespace mir::tests::unit


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
