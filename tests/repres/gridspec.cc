// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <memory>
#include <string>
#include <vector>

#include "eckit/geo/Grid.h"
#include "eckit/geo/PointLonLat.h"
#include "eckit/geo/area/BoundingBox.h"
#include "eckit/geo/eckit_geo_config.h"
#include "eckit/testing/Test.h"

#include "mir/api/MIRJob.h"
#include "mir/input/GribFileInput.h"
#include "mir/input/MIRInput.h"
#include "mir/input/RawInput.h"
#include "mir/output/ArrayOutput.h"
#include "mir/output/EmptyOutput.h"
#include "mir/param/GridSpecParametrisation.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/repres/Iterator.h"
#include "mir/repres/Representation.h"


namespace mir::repres::tests {


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

            RepresentationHandle rep(RepresentationFactory::build(*param));
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

            RepresentationHandle repres(RepresentationFactory::build(meta));

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


CASE("GridSpec regional outputs keep their grid (not unstructured)") {
    param::GridSpecParametrisation meta("{grid: 10/10}");
    std::vector<double> values(meta.grid().size(), 0.);

    auto check = [&](const std::string& gridspec, const std::string& type) {
        output::ArrayOutput output;
        api::MIRJob job;
        job.set("grid", gridspec);
        job.set("interpolation", "nn");

        for (std::unique_ptr<input::MIRInput> input(new input::RawInput(values.data(), values.size(), meta));
             input->next();) {
            job.execute(*input, output);
        }

        std::unique_ptr<const eckit::geo::Grid> expected(eckit::geo::GridFactory::make_from_string(gridspec));
        std::unique_ptr<const eckit::geo::Grid> result(eckit::geo::GridFactory::make_from_string(output.gridspec()));
        EXPECT(result->type() == type);
        EXPECT(*result == *expected);
        EXPECT(output.shape() == expected->shape());
        EXPECT(output.size() == expected->size());
    };

    SECTION("rotated_ll") {
        check(R"({"area":[3.36,-6.82,-4.42,4.8],"grid":[0.02,0.02],"order":"i+j+",)"
              R"("projection":{"south_pole":[10,-43],"type":"rotation"}})",
              "regular_ll");
    }

#if eckit_HAVE_PROJ
    SECTION("regular_xy (swisslv95)") {
        check("{type: swisslv95, x: [2480000, 2840000, 20000], y: [1080000, 1300000, 20000]}", "regular_xy");
    }
#endif
}


const std::vector<std::string> GRIDS{"grid: 10/10", "grid: 10/10, area: [60, -10, 30, 40]", "grid: F8"};
const std::vector<std::string> ORDERS{"i+j-", "i+j+", "i-j-", "i-j+", "j-i+", "j+i+", "j-i-", "j+i-"};


std::vector<std::string> gridspecs_with_orders() {
    std::vector<std::string> gridspecs;
    for (const auto& grid : GRIDS) {
        for (const auto& order : ORDERS) {
            gridspecs.emplace_back("{" + grid + ", order: " + order + "}");
        }
    }
    return gridspecs;
}


bool same_points(const std::vector<double>& lats1, const std::vector<double>& lons1, const std::vector<double>& lats2,
                 const std::vector<double>& lons2) {
    if (lats1.size() != lats2.size() || lons1.size() != lons2.size() || lats1.size() != lons1.size()) {
        return false;
    }

    for (size_t i = 0; i < lats1.size(); ++i) {
        if (!eckit::geo::points_equal(eckit::geo::PointLonLat{lons1[i], lats1[i]},
                                      eckit::geo::PointLonLat{lons2[i], lats2[i]})) {
            return false;
        }
    }

    return true;
}


CASE("GridSpec representation round trip (points, and spec describing the same grid)") {
    auto gridspecs = gridspecs_with_orders();
    gridspecs.insert(
        gridspecs.end(),
        {
            "{grid: [2, 2], area: [60, -10, 30, 40]}",
            R"({"area":[3.4,-6.8,-4.4,4.8],"grid":[0.2,0.2],"order":"i+j+","projection":{"south_pole":[10,-43],"type":"rotation"}})",
            "{grid: F16}",
            "{grid: F16, area: [60, -10, 30, 40]}",
            "{grid: O16}",
            "{grid: N32}",
            "{pl: [20, 24, 24, 20]}",
            "{grid: O16, area: [60, -10, 30, 40]}",
            "{grid: H4}",
            "{grid: H4, order: nested}",
            "{type: reduced_ll, pl: [4, 8, 12, 8, 4]}",
            "{type: reduced_ll, pl: [8, 12, 8], area: [45, 0, -45, 360]}",
        });

    for (const auto& gridspec : gridspecs) {
        SECTION(gridspec) {
            std::unique_ptr<const eckit::geo::Grid> grid(eckit::geo::GridFactory::make_from_string(gridspec));
            param::GridSpecParametrisation param(gridspec);
            RepresentationHandle repres(RepresentationFactory::build(param));

            auto latlons = grid->to_latlons();
            auto& lats   = latlons.first;
            auto& lons   = latlons.second;
            repres->reorderToCanonical(lats);
            repres->reorderToCanonical(lons);

            std::vector<double> repres_lats;
            std::vector<double> repres_lons;
            for (std::unique_ptr<Iterator> it(repres->iterator()); it->next();) {
                repres_lats.push_back((*(*it))[0]);
                repres_lons.push_back((*(*it))[1]);
            }

            EXPECT_EQUAL(repres->numberOfPoints(), grid->size());
            EXPECT(same_points(repres_lats, repres_lons, lats, lons));

            std::unique_ptr<const eckit::geo::Grid> spec_grid(
                eckit::geo::GridFactory::make_from_string(repres->spec().str()));
            EXPECT_EQUAL(spec_grid->type(), grid->type());

            const auto a = spec_grid->to_latlons();
            const auto b = grid->to_latlons();
            EXPECT(same_points(a.first, a.second, b.first, b.second));
        }
    }
}


CASE("GridSpec output values follow the output grid order") {
    auto point_values = [](const eckit::geo::Grid& grid) {
        const auto latlons = grid.to_latlons();
        const auto& lats   = latlons.first;
        const auto& lons   = latlons.second;

        std::vector<double> values(lats.size());
        for (size_t i = 0; i < values.size(); ++i) {
            values[i] = lats[i] * 1000. + lons[i];
        }
        return values;
    };

    for (const auto& grid : GRIDS) {
        param::GridSpecParametrisation canonical("{" + grid + "}");
        auto values = point_values(canonical.grid());

        for (const auto& order : ORDERS) {
            const auto gridspec = "{" + grid + ", order: " + order + "}";

            SECTION(gridspec) {
                output::ArrayOutput output;
                api::MIRJob job;
                job.set("grid", gridspec);
                job.set("interpolation", "nn");

                for (std::unique_ptr<input::MIRInput> input(
                         new input::RawInput(values.data(), values.size(), canonical));
                     input->next();) {
                    job.execute(*input, output);
                }

                std::unique_ptr<const eckit::geo::Grid> result(
                    eckit::geo::GridFactory::make_from_string(output.gridspec()));
                EXPECT(output.values() == point_values(*result));
            }
        }
    }
}


CASE("GridSpec different routings") {
    const std::vector<std::string> global{"{grid: [1,1]}", "{grid: 1/1}"};
    for (const auto& gs : global) {
        param::GridSpecParametrisation param(gs);
        std::unique_ptr<const eckit::geo::Grid> grid(eckit::geo::GridFactory::make_from_string(gs));
        ASSERT(grid);

        const std::vector<size_t> expected_shape{181, 360};
        const std::string expected_gs = R"({"grid":[1,1]})";

        EXPECT(grid->spec_str() == expected_gs);
        EXPECT(grid->shape() == expected_shape);
        EXPECT(param.spec().str() == expected_gs);
    }

    const std::vector<std::string> shifted{"{grid: 0.05/0.05, area: [89.975,-179.975,-89.975,179.975]}",
                                           "{grid: [0.05, 0.05], area: 89.975/-179.975/-89.975/179.975}"};
    for (const auto& gs : shifted) {
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


CASE("GridSpec of reduced Gaussian grids, the pl rows in the grid") {
    auto check = [](const Representation& repres, const std::string& grid, bool pl) {
        size_t count = 0;
        for (std::unique_ptr<Iterator> it(repres.iterator()); it->next();) {
            ++count;
        }

        EXPECT_EQUAL(repres.numberOfPoints(), repres.grid().size());
        EXPECT_EQUAL(count, repres.grid().size());

        const auto spec = repres.grid().spec_str();
        EXPECT(spec.find("\"grid\":\"" + grid + "\"") != std::string::npos);
        const bool has_pl = spec.find("\"pl\"") != std::string::npos;
        EXPECT_EQUAL(has_pl, pl);
    };


    SECTION("regional, as from GRIB (pl of the rows in the area, zero elsewhere)") {
        // MIR-587
        input::GribFileInput input("MIR-587.grib2");
        ASSERT(input.next());

        const input::MIRInput& in = input;
        RepresentationHandle repres(RepresentationFactory::build(in.parametrisation()));

        EXPECT_EQUAL(repres->numberOfPoints(), 3436);
        check(*repres, "O640", false);
    }


    SECTION("global, of a pl neither octahedral nor classical") {
        param::SimpleParametrisation param;
        param.set("gridType", "reduced_gg");
        param.set("N", 2L);
        param.set("pl", std::vector<long>{12, 16, 16, 12});
        param.set("north", 90.);
        param.set("west", 0.);
        param.set("south", -90.);
        param.set("east", 360.);

        RepresentationHandle repres(RepresentationFactory::build(param));

        EXPECT_EQUAL(repres->numberOfPoints(), 56);
        check(*repres, "N2", true);
    }
}


CASE("GridSpec of irregular lat/lon grids (eckit::geo irregular_ll)") {
    struct test_t {
        std::vector<double> latitudes;
        std::vector<double> longitudes;
        std::string order;
    };

    const std::vector<test_t> all_values{
        test_t{{60., 50., 45., 30.}, {-10., 0., 15.}, ""},
        test_t{{30., 45., 50., 60.}, {-10., 0., 15.}, "i+j+"},
        test_t{{60., 50., 45., 30.}, {15., 0., -10.}, "i-j-"},
    };
    for (const auto& t : all_values) {
        param::SimpleParametrisation param;
        param.set("gridded", true);
        param.set("gridType", "irregular_latlon");
        param.set("latitudes", t.latitudes);
        param.set("longitudes", t.longitudes);

        RepresentationHandle repres(RepresentationFactory::build(param));

        const auto& grid = repres->grid();
        EXPECT_EQUAL(grid.type(), "irregular_ll");
        EXPECT_EQUAL(repres->numberOfPoints(), t.latitudes.size() * t.longitudes.size());

        const auto spec  = grid.spec_str();
        const bool order = spec.find("\"order\":\"" + t.order + "\"") != std::string::npos;
        EXPECT(order == !t.order.empty());

        // the points as stored, in the directions given
        size_t n = 0;
        for (std::unique_ptr<Iterator> it(repres->iterator()); it->next(); ++n) {
            const auto& p = it->pointUnrotated();
            EXPECT_EQUAL(it->index(), n);
            EXPECT_EQUAL(p.lat().value(), t.latitudes[n / t.longitudes.size()]);
            EXPECT_EQUAL(p.lon().value(), t.longitudes[n % t.longitudes.size()]);
        }
        EXPECT_EQUAL(n, repres->numberOfPoints());

        // the same grid, by its spec
        std::unique_ptr<const eckit::geo::Grid> same(eckit::geo::GridFactory::make_from_string(spec));
        EXPECT(*same == grid);

        // an interpolation from it
        const size_t size = repres->numberOfPoints();
        std::vector<double> values(size);
        for (size_t i = 0; i < size; ++i) {
            values[i] = static_cast<double>(i);
        }

        input::RawInput input(values.data(), values.size(), param);
        output::ArrayOutput output;

        api::MIRJob job;
        job.set("grid", std::vector<double>{5., 5.});
        job.set("area", std::vector<double>{55., -5., 35., 10.});
        job.set("interpolation", "nn");
        job.execute(input, output);

        EXPECT_EQUAL(output.size(), 5 * 4);
    }
}


}  // namespace mir::repres::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
