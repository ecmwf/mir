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

#include "eckit/geo/Grid.h"
#include "eckit/geo/area/BoundingBox.h"
#include "eckit/testing/Test.h"

#include "mir/api/MIRJob.h"
#include "mir/input/MIRInput.h"
#include "mir/input/RawInput.h"
#include "mir/output/EmptyOutput.h"
#include "mir/param/GridSpecParametrisation.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/repres/Representation.h"


namespace mir::tests::unit {


CASE("GridSpec input/output") {
    output::EmptyOutput output;

    struct test_t {
        std::string grid;
        std::string canonical;
        size_t size;
        bool croppable;
    };

    std::vector<test_t> tests{
        test_t{"{grid: eORCA1_T}", R"({"grid":"eORCA1_T"})", 120184, false},  // NOTE: ORCA is non-croppable
        {"{grid: 10/10}", R"({"grid":[10,10]})", 684, true},                  //
        {"{grid: [20, 10]}", R"({"grid":[20,10]})", 342, true},               //
        {"{pl: [20, 24, 24, 20]}", R"({"grid":"O2"})", 88, true},             //
        {"{grid: o8}", R"({"grid":"O8"})", 544, true},                        //
        {"{grid: h2_ring}", R"({"grid":"H2"})", 48, false},                   // NOTE: HEALPix is non-croppable
        {"{grid: h2n}", R"({"grid":"H2","order":"nested"})", 48, false},      // NOTE: HEALPix is non-croppable
    };


    SECTION("GridSpec canonical") {
        for (const auto& test : tests) {
            std::unique_ptr<param::MIRParametrisation> param(new param::GridSpecParametrisation(test.grid));
            ASSERT(param);

            std::unique_ptr<const eckit::geo::Grid> grid(eckit::geo::GridFactory::make_from_string(test.grid));
            EXPECT(grid->size() == test.size);
            EXPECT(grid->spec_str() == test.canonical);

            static const auto bbox_spec_str = eckit::geo::area::BoundingBox::bounding_box_default().spec_str();
            EXPECT(grid->boundingBox().spec_str() == bbox_spec_str);

            repres::RepresentationHandle rep(repres::RepresentationFactory::build(*param));
            EXPECT(rep->numberOfPoints() == test.size);
            EXPECT(rep->isGlobal());
        }
    }


    SECTION("GridSpec as input") {
        for (const auto& test_input : tests) {
            param::GridSpecParametrisation meta(test_input.grid);

            api::MIRJob job;
            job.set("grid", std::vector<double>{5., 5.});

            std::vector<double> values(test_input.size, 0.);
            for (std::unique_ptr<input::MIRInput> input(new input::RawInput(values.data(), values.size(), meta));
                 input->next();) {
                job.execute(*input, output);
            }
        }
    }


    SECTION("GridSpec as output") {
        param::SimpleParametrisation meta;
        meta.set("gridded", true);
        meta.set("gridType", "regular_ll");
        meta.set("north", 20.);
        meta.set("west", 0.);
        meta.set("south", 0.);
        meta.set("east", 10.);
        meta.set("south_north_increment", 5.);
        meta.set("west_east_increment", 5.);
        meta.set("Ni", 3);
        meta.set("Nj", 5);

        for (const auto& test_output : tests) {
            if (!test_output.croppable) {
                continue;
            }

            api::MIRJob job;
            job.set("grid", test_output.grid);

            std::vector<double> values(15, 0.);
            for (std::unique_ptr<input::MIRInput> input(new input::RawInput(values.data(), values.size(), meta));
                 input->next();) {
                job.execute(*input, output);
            }
        }
    }


    SECTION("GridSpec as input and output") {
        for (const auto& test_input : tests) {
            param::GridSpecParametrisation meta(test_input.grid);

            for (const auto& test_output : tests) {
                api::MIRJob job;
                job.set("grid", test_output.grid);

                std::vector<double> values(test_input.size, 0.);
                for (std::unique_ptr<input::MIRInput> input(new input::RawInput(values.data(), values.size(), meta));
                     input->next();) {
                    job.execute(*input, output);
                }
            }
        }
    }
}


}  // namespace mir::tests::unit


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
