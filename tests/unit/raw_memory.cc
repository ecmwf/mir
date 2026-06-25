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


#include <algorithm>
#include <memory>
#include <sstream>
#include <vector>

#include "eckit/exception/Exceptions.h"
#include "eckit/filesystem/PathName.h"
#include "eckit/testing/Test.h"
#include "eckit/types/FloatCompare.h"

#include "mir/api/MIRJob.h"
#include "mir/data/MIRField.h"
#include "mir/input/RawInput.h"
#include "mir/method/WeightMatrix.h"
#include "mir/output/ArrayOutput.h"
#include "mir/output/RawOutput.h"
#include "mir/output/ResizableOutput.h"
#include "mir/param/GridSpecParametrisation.h"
#include "mir/param/RuntimeParametrisation.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/repres/Representation.h"
#include "mir/util/Log.h"


namespace mir::tests::unit {


CASE("Interfacing 1") {
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

        const repres::RepresentationHandle rep(field.representation());
        ASSERT(rep);
        log << *rep << std::endl;
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

        const repres::RepresentationHandle rep(field.representation());
        ASSERT(rep);
        log << *rep << std::endl;
    }


    SECTION("sh truncation=21") {
        // metadata
        param::SimpleParametrisation meta;

        meta.set("spectral", true);
        meta.set("gridType", "sh");
        meta.set("truncation", 21);


        // data
        std::vector<double> values(506, 0.);
        std::unique_ptr<input::MIRInput> input(new input::RawInput(values.data(), values.size(), meta));


        // access a field (in the post-processing context)
        log << *input << std::endl;

        auto field = input->field();
        log << field << std::endl;

        const repres::RepresentationHandle rep(field.representation());
        ASSERT(rep);
        log << *rep << std::endl;
    }


    SECTION("grid=ORCA2_T") {
        // metadata
        param::SimpleParametrisation meta;

        meta.set("gridded", true);
        meta.set("gridType", "orca");
        meta.set("uid", "d5bde4f52ff3a9bea5629cd9ac514410");


        // data
        std::vector<double> values(27118, 0.);  // 182 * 149
        std::unique_ptr<input::MIRInput> input(new input::RawInput(values.data(), values.size(), meta));


        // access a field (in the post-processing context)
        log << *input << std::endl;

        auto field = input->field();
        log << field << std::endl;

        const repres::RepresentationHandle rep(field.representation());
        ASSERT(rep);
        log << *rep << std::endl;
    }
}


CASE("Interfacing 2") {
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

    param::SimpleParametrisation meta1;
    meta1.set("gridded", true);
    meta1.set("gridType", "reduced_gg");
    meta1.set("north", 90.);
    meta1.set("west", 0.);
    meta1.set("south", -90.);
    meta1.set("east", 360.);
    meta1.set("N", 4);
    meta1.set("pl", std::vector<long>{20, 24, 28, 32, 32, 28, 24, 20});

    std::vector<double> values1(208 /*sum(pl)*/, 0.);
    std::fill_n(values1.begin() + 20 + 24 + 28, 32, 42.);
    std::fill_n(values1.begin() + 20 + 24 + 28 + 32, 32, -42.);

    std::unique_ptr<input::MIRInput> input(new input::RawInput(values1.data(), values1.size(), meta1));


    // job
    api::MIRJob job;
    job.set("grid", std::vector<double>{2., 2.});
    job.set("area", std::vector<double>{1., -1., -1., 1.});
    job.set("interpolation", "nn");
    job.set("caching", false);

    log << job << std::endl;


    SECTION("process with output of static size") {
        // output
        param::SimpleParametrisation meta2;
        std::vector<double> values2(4, 0);
        std::unique_ptr<output::MIROutput> output(new output::RawOutput(values2.data(), values2.size(), meta2));


        // process
        job.execute(*input, *output);

        EXPECT_EQUAL(values2[0], 42.);
        EXPECT_EQUAL(values2[1], 42.);
        EXPECT_EQUAL(values2[2], -42.);
        EXPECT_EQUAL(values2[3], -42.);

        log << "output metadata: " << meta2 << std::endl;

        std::ostringstream ss;
        ss << meta2;
        EXPECT(ss.str() == R"({"grid":"{\"area\":[1,-1,-1,1],\"grid\":[2,2],\"reference\":[1,1]}"})");
    }


    SECTION("process with output of dynamic size") {
        // output (RawOutput instead of MIROutput to access specific methods)
        param::SimpleParametrisation meta2;
        std::vector<double> values2;
        std::unique_ptr<output::MIROutput> output(new output::ResizableOutput(values2, meta2));


        // process
        job.execute(*input, *output);

        EXPECT(values2.size() == 4);

        EXPECT_EQUAL(values2[0], 42.);
        EXPECT_EQUAL(values2[1], 42.);
        EXPECT_EQUAL(values2[2], -42.);
        EXPECT_EQUAL(values2[3], -42.);

        log << "output metadata: " << meta2 << std::endl;

        std::ostringstream ss;
        ss << meta2;
        EXPECT(ss.str() == R"({"grid":"{\"area\":[1,-1,-1,1],\"grid\":[2,2],\"reference\":[1,1]}"})");
    }
}


CASE("Interfacing 3") {
    auto& log = Log::info();


    // input, with a values vector representing spherical harmonics scalar field
    param::SimpleParametrisation meta1;
    meta1.set("spectral", true);
    meta1.set("gridType", "sh");
    meta1.set("truncation", 21);

    std::vector<double> values1(506, 0.);
    std::unique_ptr<input::MIRInput> input(new input::RawInput(values1.data(), values1.size(), meta1));


    // job
    api::MIRJob job;
    job.set("grid", std::vector<double>{2., 2.});
    job.set("area", std::vector<double>{1., -1., -1., 1.});
    job.set("caching", false);


    SECTION("process with output of static size") {
        // output
        param::SimpleParametrisation meta2;
        std::vector<double> values2(4, 0);
        std::unique_ptr<output::MIROutput> output(new output::RawOutput(values2.data(), values2.size(), meta2));

        log << job << std::endl;


        // process
        job.execute(*input, *output);

        EXPECT_EQUAL(values2[0], 0.);  // TODO: improve results check
        EXPECT_EQUAL(values2[1], 0.);
        EXPECT_EQUAL(values2[2], 0.);
        EXPECT_EQUAL(values2[3], 0.);

        log << "output metadata: " << meta2 << std::endl;

        std::ostringstream ss;
        ss << meta2;
        EXPECT(ss.str() == R"({"grid":"{\"area\":[1,-1,-1,1],\"grid\":[2,2],\"reference\":[1,1]}"})");
    }


    SECTION("process with output of dynamic size") {
        // output (RawOutput instead of MIROutput to access specific methods)
        param::SimpleParametrisation meta2;
        std::vector<double> values2;
        std::unique_ptr<output::MIROutput> output(new output::ResizableOutput(values2, meta2));


        // process
        job.execute(*input, *output);

        EXPECT(values2.size() == 4);

        EXPECT_EQUAL(values2[0], 0.);
        EXPECT_EQUAL(values2[1], 0.);
        EXPECT_EQUAL(values2[2], 0.);
        EXPECT_EQUAL(values2[3], 0.);

        log << "output metadata: " << meta2 << std::endl;

        std::ostringstream ss;
        ss << meta2;
        EXPECT(ss.str() == R"({"grid":"{\"area\":[1,-1,-1,1],\"grid\":[2,2],\"reference\":[1,1]}"})");
    }
}


CASE("Interfacing 4") {
    auto& log = Log::info();


    SECTION("grid=unstructured regridded") {
        const double missingValue    = 42.;
        const eckit::PathName matrix = "raw_memory_example_3.mat";


        // input metadata & data
        param::SimpleParametrisation meta1;

        meta1.set("gridded", true);
        meta1.set("gridType", "unstructured_grid");
        meta1.set("numberOfPoints", 3);
        meta1.set("missing_value", missingValue);

        std::vector<double> values1(3, 1.);
        std::unique_ptr<input::MIRInput> input(new input::RawInput(values1.data(), values1.size(), meta1));


        // output metadata (empty) & data (resizable)
        param::SimpleParametrisation meta2;
        std::vector<double> values2;
        std::unique_ptr<output::MIROutput> output(new output::ResizableOutput(values2, meta2));


        // matrix file
        if (!matrix.exists()) {
            method::WeightMatrix W(4, 3);
            W.setFromTriplets({{0, 1, 0.4}, {0, 2, 0.6}, {1, 0, 0.1}, {1, 1, 0.1}, {1, 2, 0.8}, {2, 2, 1.}});
            W.save(matrix);
        }


        // job
        api::MIRJob job;
        job.set("grid", std::vector<double>{2., 2.});
        job.set("area", std::vector<double>{1., -1., -1., 1.});
        job.set("interpolation", "matrix");
        job.set("interpolation-matrix", matrix);

        log << job << std::endl;


        // process
        job.execute(*input, *output);

        EXPECT(values2.size() == 4);

        EXPECT_EQUAL(values2[0], 1.);
        EXPECT_EQUAL(values2[1], 1.);
        EXPECT_EQUAL(values2[2], 1.);
        EXPECT_EQUAL(values2[3], missingValue);

        log << "output metadata: " << meta2 << std::endl;

        std::ostringstream ss;
        ss << meta2;
        EXPECT(ss.str() ==
               R"({"grid":"{\"area\":[1,-1,-1,1],\"grid\":[2,2],\"reference\":[1,1]}","missing_value":42})");
    }
}


CASE("Interfacing 5") {
    const std::string out_grid = "{\"grid\":[1, 1]}";
    const double missing_value = 42.;


    // input metadata & data
    param::GridSpecParametrisation in_grid("{grid: [2, 2], area: [20, 1, 1, 20]}");
    param::RuntimeParametrisation meta1(in_grid);

    std::vector<size_t> in_shape{10, 10};
    std::vector<double> values1(in_shape[0] * in_shape[1], 1.);
    std::unique_ptr<input::MIRInput> input(new input::RawInput(values1.data(), values1.size(), meta1));


    // confirm input grid is as expected
    std::unique_ptr<const eckit::geo::Grid> grid1(
        eckit::geo::GridFactory::build(repres::RepresentationHandle(input->field().representation())->spec()));

    EXPECT(grid1->shape() == in_shape);
    EXPECT(grid1->size() == in_shape[0] * in_shape[1]);
    EXPECT(grid1->spec_str() == R"({"area":[19,1,1,19],"grid":[2,2],"reference":[1,1]})");


    // output metadata (empty) & data (resizable)
    std::vector<double> values2;
    param::SimpleParametrisation meta2;
    std::unique_ptr<output::MIROutput> output(new output::ResizableOutput(values2, meta2));


    // job (1)
    api::MIRJob job;
    job.set("grid", out_grid);


    // process (1)
    job.execute(*input, *output);


    // output (1)
    const std::string out_spec_str = R"({"area":[19,1,1,19],"grid":[1,1]})";
    const std::vector<size_t> out_shape{19, 19};

    std::unique_ptr<const eckit::geo::Grid> grid2(eckit::geo::GridFactory::make_from_string([&meta2]() {
        std::string grid;
        meta2.get("grid", grid);
        return grid;
    }()));


    EXPECT(out_spec_str == grid2->spec_str());
    EXPECT(out_shape == grid2->shape());
    EXPECT(values2.size() == grid2->size());

    EXPECT_NOT(meta2.has("missing_value"));


    // input (2)
    meta1.set("missing_value", missing_value);
    values1[3] = missing_value;


    // process (2)
    job.execute(*input, *output);


    // output (2)
    std::unique_ptr<const eckit::geo::Grid> grid3(eckit::geo::GridFactory::make_from_string([&meta2]() {
        std::string grid;
        meta2.get("grid", grid);
        return grid;
    }()));

    EXPECT(*grid3 == *grid2);
    EXPECT(meta2.has("missing_value"));
    EXPECT(std::any_of(values2.begin(), values2.end(),
                       [&missing_value](double v) { return eckit::types::is_approximately_equal(v, missing_value); }));
}


CASE("Interfacing 6") {
    const std::string out_grid = "{\"grid\":[1, 1]}";
    const double missing_value = 42.;


    // input metadata & data
    param::GridSpecParametrisation in_grid("{grid: [2, 2], area: [20, 1, 1, 20]}");
    param::RuntimeParametrisation meta1(in_grid);
    meta1.set("missing_value", missing_value);

    std::vector<size_t> in_shape{10, 10};
    std::vector<double> values1(in_shape[0] * in_shape[1], 1.);
    values1[3] = missing_value;

    std::unique_ptr<input::MIRInput> input(new input::RawInput(values1.data(), values1.size(), meta1));


    // confirm input grid is as expected
    std::unique_ptr<const eckit::geo::Grid> grid1(
        eckit::geo::GridFactory::build(repres::RepresentationHandle(input->field().representation())->spec()));

    EXPECT(grid1->shape() == in_shape);
    EXPECT(grid1->size() == in_shape[0] * in_shape[1]);
    EXPECT(grid1->spec_str() == R"({"area":[19,1,1,19],"grid":[2,2],"reference":[1,1]})");


    // output
    output::ArrayOutput output;


    // job
    api::MIRJob job;
    job.set("grid", out_grid);


    // process
    job.execute(*input, output);


    // output
    const std::string out_spec_str = R"({"area":[19,1,1,19],"grid":[1,1]})";
    const std::vector<size_t> out_shape{19, 19};

    std::unique_ptr<const eckit::geo::Grid> grid2(eckit::geo::GridFactory::make_from_string(output.gridspec()));


    EXPECT(out_spec_str == grid2->spec_str());
    EXPECT(out_shape == grid2->shape());
    EXPECT(output.values().size() == grid2->size());
    EXPECT(eckit::types::is_approximately_equal(output.missingValue(), missing_value));
}


}  // namespace mir::tests::unit


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
