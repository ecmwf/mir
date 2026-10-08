// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <algorithm>
#include <cmath>
#include <memory>
#include <string>
#include <vector>

#include "eckit/geo/Grid.h"
#include "eckit/geo/area/BoundingBox.h"
#include "eckit/testing/Test.h"

#include "mir/api/MIRJob.h"
#include "mir/input/MIRInput.h"
#include "mir/input/RawInput.h"
#include "mir/output/ArrayOutput.h"
#include "mir/output/EmptyOutput.h"
#include "mir/param/GridSpecParametrisation.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/repres/Iterator.h"
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
        {"{grid: hr2}", R"({"grid":"H2"})", 48, false},                       // NOTE: HEALPix is non-croppable
        {"{grid: h2n}", R"({"grid":"H2","order":"nested"})", 48, false},      // NOTE: HEALPix is non-croppable
        {"{grid: o96}", R"({"grid":"O96"})", 40320, false},
        // {R"({"area":[89.2842275325138,0,-89.2842275325138,359.1],"grid":"O96"})", R"({"grid":"O96"})", 40320, false},
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
            job.set("interpolation", "nn");

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
            job.set("interpolation", "nn");

            std::vector<double> values(15, 0.);
            for (std::unique_ptr<input::MIRInput> input(new input::RawInput(values.data(), values.size(), meta));
                 input->next();) {
                job.execute(*input, output);
            }
        }
    }


    SECTION("GridSpec as input and output") {
        for (const auto& test : tests) {
            param::GridSpecParametrisation meta(test.grid);

            EXPECT(meta.grid().size() == test.size);
            EXPECT(meta.spec().str() == test.canonical);

            repres::RepresentationHandle repres(repres::RepresentationFactory::build(meta));

            EXPECT(repres->spec().str() == test.canonical);

            for (const auto& test_output : tests) {
                output::ArrayOutput output;
                api::MIRJob job;
                job.set("grid", test_output.grid);
                job.set("interpolation", "nn");

                std::vector<double> values(test.size, 0.);
                for (std::unique_ptr<input::MIRInput> input(new input::RawInput(values.data(), values.size(), meta));
                     input->next();) {
                    job.execute(*input, output);
                }

                EXPECT(output.gridspec() == test_output.canonical);
                EXPECT(output.size() == test_output.size);
            }
        }
    }
}


bool same_points(const std::vector<double>& lats1, const std::vector<double>& lons1, const std::vector<double>& lats2,
                 const std::vector<double>& lons2) {
    constexpr double EPS = 1e-9;

    auto same_lon = [](double a, double b) {
        auto d = std::fmod(std::abs(a - b), 360.);
        return std::min(d, 360. - d) < EPS;
    };

    if (lats1.size() != lats2.size() || lons1.size() != lons2.size() || lats1.size() != lons1.size()) {
        return false;
    }

    for (size_t i = 0; i < lats1.size(); ++i) {
        if (std::abs(lats1[i] - lats2[i]) > EPS || !same_lon(lons1[i], lons2[i])) {
            return false;
        }
    }

    return true;
}


CASE("GridSpec representation round trip (points, and spec describing the same grid)") {
    const std::vector<std::string> gridspecs{
        "{grid: 10/10}",
        "{grid: [2, 2], area: [60, -10, 30, 40]}",
        "{grid: F16}",
        "{grid: F16, area: [60, -10, 30, 40]}",
        "{grid: O16}",
        "{grid: N32}",
        "{pl: [20, 24, 24, 20]}",
        "{grid: O16, area: [60, -10, 30, 40]}",
        "{grid: H4}",
        "{grid: H4, order: nested}",
    };

    for (const auto& gridspec : gridspecs) {
        SECTION(gridspec) {
            std::unique_ptr<const eckit::geo::Grid> grid(eckit::geo::GridFactory::make_from_string(gridspec));
            param::GridSpecParametrisation param(gridspec);
            repres::RepresentationHandle repres(repres::RepresentationFactory::build(param));

            const auto [lats, lons] = grid->to_latlons();

            std::vector<double> repres_lats;
            std::vector<double> repres_lons;
            for (std::unique_ptr<repres::Iterator> it(repres->iterator()); it->next();) {
                repres_lats.push_back((*(*it))[0]);
                repres_lons.push_back((*(*it))[1]);
            }

            EXPECT_EQUAL(repres->numberOfPoints(), grid->size());
            EXPECT(same_points(repres_lats, repres_lons, lats, lons));

            std::unique_ptr<const eckit::geo::Grid> spec_grid(
                eckit::geo::GridFactory::make_from_string(repres->spec().str()));
            const auto [spec_lats, spec_lons] = spec_grid->to_latlons();
            const auto [grid_lats, grid_lons] = grid->to_latlons();
            EXPECT(same_points(spec_lats, spec_lons, grid_lats, grid_lons));
        }
    }
}


CASE("GridSpec different routings") {
    for (const auto* gs : {
             "{grid: [1,1]}",
             "{grid: 1/1}",
         }) {
        param::GridSpecParametrisation param(gs);
        std::unique_ptr<const eckit::geo::Grid> grid(eckit::geo::GridFactory::make_from_string(gs));
        ASSERT(grid);

        const std::vector<size_t> expected_shape{181, 360};
        const std::string expected_gs = R"({"grid":[1,1]})";

        EXPECT(grid->spec_str() == expected_gs);
        EXPECT(grid->shape() == expected_shape);
        EXPECT(param.spec().str() == expected_gs);
    }

    for (const auto* gs : {
             "{grid: 0.05/0.05, area: [89.975,-179.975,-89.975,179.975]}",
             "{grid: [0.05, 0.05], area: 89.975/-179.975/-89.975/179.975}",
         }) {
        param::GridSpecParametrisation param(gs);
        std::unique_ptr<const eckit::geo::Grid> grid(eckit::geo::GridFactory::make_from_string(gs));
        ASSERT(grid);

        const std::vector<size_t> expected_shape{3600, 7200};
        const std::string expected_gs =
            R"({"area":[90,-179.975,-90,180.025],"grid":[0.05,0.05],"reference":[0.025,0.025]})";

        EXPECT(grid->spec_str() == expected_gs);
        EXPECT(grid->shape() == expected_shape);
        EXPECT(param.spec().str() == expected_gs);
    }
}


}  // namespace mir::tests::unit


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
