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


#include <initializer_list>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "eckit/filesystem/PathName.h"
#include "eckit/geo/Grid.h"
#include "eckit/parser/YAMLParser.h"
#include "eckit/testing/Test.h"

#include "mir/api/MIRJob.h"
#include "mir/caching/WeightCache.h"
#include "mir/input/RawInput.h"
#include "mir/output/ArrayOutput.h"
#include "mir/output/EmptyOutput.h"
#include "mir/param/GridSpecParametrisation.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/repres/Representation.h"
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


    static const auto version = std::to_string(mir::caching::WeightCacheTraits::version());

    static const auto EXPECTED_INTERPOLATION_LINEAR{"{engine => mir , version => " + version + " , method => linear}"};
    static const auto EXPECTED_INTERPOLATION_NN{"{engine => mir , version => " + version +
                                                " , method => nearest-neighbour}"};
    static const auto EXPECTED_INTERPOLATION_GBA{"{engine => mir , version => " + version +
                                                 " , method => grid-box-average}"};


    SECTION("interpolation=linear (default)") {
        std::unique_ptr<input::MIRInput> input(new input::RawInput(values.data(), values.size(), meta));

        api::MIRJob job;
        job.set("grid", std::vector<double>{2., 2.});
        job.set("caching", false);
        job.set("interpolation", "linear");
        job.set("dump-weights-info", info_path);
        job.execute(*input, *output);

        Log::info() << info_interpolation(info_path) << std::endl;
        // EXPECT(info_interpolation(info_path) == EXPECTED_INTERPOLATION_LINEAR);
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


#if 0
    SECTION("interpolation=nn (default)") {
        std::unique_ptr<input::MIRInput> input(new input::RawInput(values.data(), values.size(), meta));

        api::MIRJob job;
        job.set("grid", std::vector<double>{2., 2.});
        job.set("caching", false);
        job.set("interpolation", "nn");
        job.set("dump-weights-info", info_path);
        job.execute(*input, *output);

        Log::info() << info_interpolation(info_path) << std::endl;
        EXPECT(info_interpolation(info_path) == EXPECTED_INTERPOLATION_NN);
    }
#endif


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


#if 0
    SECTION("interpolation=grid-box-average (default)") {
        std::unique_ptr<input::MIRInput> input(new input::RawInput(values.data(), values.size(), meta));

        api::MIRJob job;
        job.set("grid", std::vector<double>{2., 2.});
        job.set("caching", false);
        job.set("interpolation", "grid-box-average");
        job.set("dump-weights-info", info_path);
        job.execute(*input, *output);

        Log::info() << info_interpolation(info_path) << std::endl;
        EXPECT(info_interpolation(info_path) == EXPECTED_INTERPOLATION_GBA);
    }
#endif


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
    using Grid = std::unique_ptr<const eckit::geo::Grid>;

    using eckit::geo::GridFactory;


    for (const auto& test : std::initializer_list<std::pair<std::string, std::string>> {
             std::pair<std::string, std::string>{R"({grid: H2})", R"({"grid":"H2"})"},  // HEALPix ring grids
             {R"({grid: H4})", R"({"grid":"H4"})"},                                     //
             {R"({grid: h8R})", R"({"grid":"H8"})"},                                    //
             {R"({grid: Hr16})", R"({"grid":"H16"})"},                                  //
             {R"({grid: H32, order: ring})", R"({"grid":"H32"})"},                      //

             {R"({grid: H64, order: nested})", R"({"grid":"H64","order":"nested"})"},    // HEALPix nested grids
             {R"({grid: h128N})", R"({"grid":"H128","order":"nested"})"},                //
             {R"({grid: Hn256})", R"({"grid":"H256","order":"nested"})"},                //
             {R"({grid: h512, order: nested})", R"({"grid":"H512","order":"nested"})"},  //
             {R"({grid: hN1024})", R"({"grid":"H1024","order":"nested"})"},              //

             {R"({grid: F2})", R"({"grid":"F2"})"},          // Gaussian grids
             {R"({"grid": "f32"})", R"({"grid":"F32"})"},    //
             {R"({grid: o2})", R"({"grid":"O2"})"},          //
             {R"({grid: o2560})", R"({"grid":"O2560"})"},    //
             {R"({"grid": "n640"})", R"({"grid":"N640"})"},  //

             {"{grid: [0.1,0.1]}", R"({"grid":[0.1,0.1]})"},          // regular lat-lon grids
             {"{grid: [0.125,0.125]}", R"({"grid":[0.125,0.125]})"},  //
             {"{grid: [0.15,0.15]}", R"({"grid":[0.15,0.15]})"},      //
             {"{grid: [0.2,0.2]}", R"({"grid":[0.2,0.2]})"},          //
             {"{grid: [0.25,0.25]}", R"({"grid":[0.25,0.25]})"},      //
             {"{grid: [0.3,0.3]}", R"({"grid":[0.3,0.3]})"},          //
             {"{grid: [0.4,0.4]}", R"({"grid":[0.4,0.4]})"},          //
             {"{grid: [0.5,0.5]}", R"({"grid":[0.5,0.5]})"},          //
             {"{grid: [0.6,0.6]}", R"({"grid":[0.6,0.6]})"},          //
             {"{grid: [0.7,0.7]}", R"({"grid":[0.7,0.7]})"},          //
             {"{grid: [0.75,0.75]}", R"({"grid":[0.75,0.75]})"},      //
             {"{grid: [0.8,0.8]}", R"({"grid":[0.8,0.8]})"},          //
             {"{grid: [0.9,0.9]}", R"({"grid":[0.9,0.9]})"},          //
             {"{grid: [1,1]}", R"({"grid":[1,1]})"},                  //
             {"{grid: [1.2,1.2]}", R"({"grid":[1.2,1.2]})"},          //
             {"{grid: [1.25,1.25]}", R"({"grid":[1.25,1.25]})"},      //
             {"{grid: [1.4,1.4]}", R"({"grid":[1.4,1.4]})"},          //
             {"{grid: [1.5,1.5]}", R"({"grid":[1.5,1.5]})"},          //
             {"{grid: [1.6,1.6]}", R"({"grid":[1.6,1.6]})"},          //
             {"{grid: [1.8,1.8]}", R"({"grid":[1.8,1.8]})"},          //
             {"{grid: [10,10]}", R"({"grid":[10,10]})"},              //
             {"{grid: [2,2]}", R"({"grid":[2,2]})"},                  //
             {"{grid: [2.5,2.5]}", R"({"grid":[2.5,2.5]})"},          //
             {"{grid: [5,5]}", R"({"grid":[5,5]})"},                  //

             {"{grid: eORCA1_T}", R"({"grid":"eORCA1_T"})"},  // ORCA grids
             {"{grid: 16076978a048410747dd7c9876677b28}",
              R"({"grid":"eORCA1_T","uid":"16076978a048410747dd7c9876677b28"})"},  //
             {"{grid: 16076978a048410747dd7c9876677b28}",
              R"({"grid":"eORCA1_T","uid":"16076978a048410747dd7c9876677b28"})"},  //
         }) {
        Grid a(eckit::geo::GridFactory::make_from_string(test.first));

        Log::info() << test.second << " == " << a->spec_str() << std::endl;
        EXPECT(test.second == a->spec_str());

        Grid b(eckit::geo::GridFactory::build(a->spec()));
        EXPECT(*a == *b);

        Grid c(eckit::geo::GridFactory::make_from_string(b->spec_str()));
        EXPECT(*a == *c);
    }
}


CASE("regrid") {
    using Grid = std::unique_ptr<const eckit::geo::Grid>;

    using eckit::geo::GridFactory;
    using eckit::geo::PointLonLat;
    using eckit::spec::Custom;


    SECTION("Arakawa C-type grids UM <-> regular_ll") {
        struct test_t {
            size_t N;
            std::vector<size_t> shape;
            std::string spec_str;
        };

        for (const auto& test : {
                 test_t{48, {72, 96}, R"({"grid":[3.75,2.5],"order":"i+j+","reference":[1.875,1.25]})"},
                 test_t{400, {600, 800}, R"({"grid":[0.45,0.3],"order":"i+j+","reference":[0.225,0.15]})"},
             }) {
            eckit::spec::Custom spec({{"type", "arakawa_c_um"}, {"N", test.N}});

            Grid g(GridFactory::build(spec));

            EXPECT(g->shape() == test.shape);
            EXPECT(g->spec_str() == test.spec_str);

            spec.set("arrangement", "T");
            Grid t(GridFactory::build(spec));

            EXPECT(t->shape() == test.shape);

            spec.set("arrangement", "U");
            Grid u(GridFactory::build(spec));

            EXPECT(u->shape() == test.shape);

            spec.set("arrangement", "V");
            Grid v(GridFactory::build(spec));
            auto test_shape_v = test.shape;
            test_shape_v[0] += 1;

            EXPECT(v->shape() == test_shape_v);


            // test representation/interpolation for arrangements T (default), T, U and V
            for (const auto* grid : {g.get(), t.get(), u.get(), v.get()}) {
                ASSERT(grid != nullptr);

                param::GridSpecParametrisation param(grid->spec_str());
                repres::RepresentationHandle rep(repres::RepresentationFactory::build(param));

                auto v              = std::get<PointLonLat>(grid->first_point()).south_pole();
                auto numberOfPoints = (test.shape[0] + (v ? 1 : 0)) * test.shape[1];

                EXPECT(rep && rep->isGlobal());
                EXPECT(rep->numberOfPoints() == numberOfPoints);

                std::vector<double> values(grid->size(), 0.);
                std::unique_ptr<input::MIRInput> input(new input::RawInput(values.data(), values.size(), param));

                const std::vector<size_t> output_shape{19, 36};
                output::ArrayOutput output;

                api::MIRJob job;
                job.set("grid", std::vector<double>{10., 10.});
                job.execute(*input, output);

                EXPECT(output.shape() == output_shape);
                EXPECT(output.gridspec() == R"({"grid":[10,10]})");
            }
        }
    }


    SECTION("Arakawa C-type grids UM <-> reduced_gg") {
        struct Input {
            explicit Input(const std::string& spec_str) :
                param(spec_str), values(param.grid().size(), 0.), raw(values.data(), values.size(), param) {}

            const eckit::geo::Grid& grid() { return param.grid(); }
            input::MIRInput& input() { return raw; }

            param::GridSpecParametrisation param;
            std::vector<double> values;
            input::RawInput raw;
        } a("{type: arakawa_c_um, N: 96}"), b("{grid: o96}");

        output::ArrayOutput output;
        api::MIRJob job;

        job.set("grid", b.grid().spec_str());
        job.execute(a.input(), output);

        EXPECT(output.shape() == b.grid().shape());
        EXPECT(output.gridspec() == b.grid().spec_str());

        job.set("grid", a.grid().spec_str());
        job.execute(b.input(), output);

        EXPECT(output.shape() == a.grid().shape());
        EXPECT(output.gridspec() == a.grid().spec_str());
    }
}


}  // namespace mir::tests::unit


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
