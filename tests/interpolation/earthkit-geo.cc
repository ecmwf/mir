// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <memory>
#include <string>
#include <vector>

#include "eckit/geo/Grid.h"
#include "eckit/testing/Test.h"

#include "mir/api/MIRJob.h"
#include "mir/input/RawInput.h"
#include "mir/output/ArrayOutput.h"
#include "mir/param/GridSpecParametrisation.h"
#include "mir/repres/Representation.h"
#include "mir/util/Log.h"


namespace mir::interpolation::tests {


CASE("GridSpec") {
    using Grid = std::unique_ptr<const eckit::geo::Grid>;

    using eckit::geo::GridFactory;


    struct test_t {
        std::string spec;
        std::string expected;
    };

    for (const auto& test : {
             test_t{R"({grid: H2})", R"({"grid":"H2"})"},                 // HEALPix ring grids
             test_t{R"({grid: H4})", R"({"grid":"H4"})"},                 //
             test_t{R"({grid: h8R})", R"({"grid":"H8"})"},                //
             test_t{R"({grid: Hr16})", R"({"grid":"H16"})"},              //
             test_t{R"({grid: H32, order: ring})", R"({"grid":"H32"})"},  //

             test_t{R"({grid: H64, order: nested})", R"({"grid":"H64","order":"nested"})"},    // HEALPix nested grids
             test_t{R"({grid: h128N})", R"({"grid":"H128","order":"nested"})"},                //
             test_t{R"({grid: Hn256})", R"({"grid":"H256","order":"nested"})"},                //
             test_t{R"({grid: h512, order: nested})", R"({"grid":"H512","order":"nested"})"},  //
             test_t{R"({grid: hN1024})", R"({"grid":"H1024","order":"nested"})"},              //

             test_t{R"({grid: F2})", R"({"grid":"F2"})"},          // Gaussian grids
             test_t{R"({"grid": "f32"})", R"({"grid":"F32"})"},    //
             test_t{R"({grid: o2})", R"({"grid":"O2"})"},          //
             test_t{R"({grid: o2560})", R"({"grid":"O2560"})"},    //
             test_t{R"({"grid": "n640"})", R"({"grid":"N640"})"},  //

             test_t{"{grid: [0.1,0.1]}", R"({"grid":[0.1,0.1]})"},          // regular lat-lon grids
             test_t{"{grid: [0.125,0.125]}", R"({"grid":[0.125,0.125]})"},  //
             test_t{"{grid: [0.15,0.15]}", R"({"grid":[0.15,0.15]})"},      //
             test_t{"{grid: [0.2,0.2]}", R"({"grid":[0.2,0.2]})"},          //
             test_t{"{grid: [0.25,0.25]}", R"({"grid":[0.25,0.25]})"},      //
             test_t{"{grid: [0.3,0.3]}", R"({"grid":[0.3,0.3]})"},          //
             test_t{"{grid: [0.4,0.4]}", R"({"grid":[0.4,0.4]})"},          //
             test_t{"{grid: [0.5,0.5]}", R"({"grid":[0.5,0.5]})"},          //
             test_t{"{grid: [0.6,0.6]}", R"({"grid":[0.6,0.6]})"},          //
             test_t{"{grid: [0.7,0.7]}", R"({"grid":[0.7,0.7]})"},          //
             test_t{"{grid: [0.75,0.75]}", R"({"grid":[0.75,0.75]})"},      //
             test_t{"{grid: [0.8,0.8]}", R"({"grid":[0.8,0.8]})"},          //
             test_t{"{grid: [0.9,0.9]}", R"({"grid":[0.9,0.9]})"},          //
             test_t{"{grid: [1,1]}", R"({"grid":[1,1]})"},                  //
             test_t{"{grid: [1.2,1.2]}", R"({"grid":[1.2,1.2]})"},          //
             test_t{"{grid: [1.25,1.25]}", R"({"grid":[1.25,1.25]})"},      //
             test_t{"{grid: [1.4,1.4]}", R"({"grid":[1.4,1.4]})"},          //
             test_t{"{grid: [1.5,1.5]}", R"({"grid":[1.5,1.5]})"},          //
             test_t{"{grid: [1.6,1.6]}", R"({"grid":[1.6,1.6]})"},          //
             test_t{"{grid: [1.8,1.8]}", R"({"grid":[1.8,1.8]})"},          //
             test_t{"{grid: [10,10]}", R"({"grid":[10,10]})"},              //
             test_t{"{grid: [2,2]}", R"({"grid":[2,2]})"},                  //
             test_t{"{grid: [2.5,2.5]}", R"({"grid":[2.5,2.5]})"},          //
             test_t{"{grid: [5,5]}", R"({"grid":[5,5]})"},                  //

             test_t{"{grid: eORCA1_T}", R"({"grid":"eORCA1_T"})"},  // ORCA grids
             test_t{"{grid: 16076978a048410747dd7c9876677b28}",
                    R"({"grid":"eORCA1_T","uid":"16076978a048410747dd7c9876677b28"})"},  //
             test_t{"{grid: 16076978a048410747dd7c9876677b28}",
                    R"({"grid":"eORCA1_T","uid":"16076978a048410747dd7c9876677b28"})"},  //
         }) {
        Grid a(eckit::geo::GridFactory::make_from_string(test.spec));

        Log::info() << test.expected << " == " << a->spec_str() << std::endl;
        EXPECT(test.expected == a->spec_str());

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


}  // namespace mir::interpolation::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
