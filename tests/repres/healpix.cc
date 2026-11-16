// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "eckit/geo/Grid.h"
#include "eckit/testing/Test.h"
#include "eckit/types/FloatCompare.h"

#include "mir/api/MIRJob.h"
#include "mir/input/GribFileInput.h"
#include "mir/input/RawInput.h"
#include "mir/key/grid/Grid.h"
#include "mir/output/ArrayOutput.h"
#include "mir/output/GribMemoryOutput.h"
#include "mir/output/ResizableOutput.h"
#include "mir/param/GridSpecParametrisation.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/repres/HEALPix.h"
#include "mir/util/GridBox.h"
#include "mir/util/MeshGeneratorParameters.h"
#include "mir/util/Rotation.h"

#include "repres.h"


namespace mir::repres::tests {


CASE("HEALPix representation") {
    RepresentationHandle ring(new HEALPix(4));
    RepresentationHandle nested(new HEALPix(4, "nested"));

    const auto& r = dynamic_cast<const HEALPix&>(*ring);
    EXPECT_EQUAL(r.Nside(), 4);
    EXPECT_EQUAL(r.order(), "ring");
    EXPECT_EQUAL(nested->order(), "nested");

    EXPECT_EQUAL(ring->numberOfPoints(), 12 * 4 * 4);
    EXPECT_EQUAL(ring->uniqueName(), "H4");
    EXPECT_EQUAL(nested->uniqueName(), "HN4");
    EXPECT_EQUAL(to_string(*nested), "HEALPix[name=HN4]");

    EXPECT_NOT(ring->sameAs(*nested));
    EXPECT(ring->sameAs(*make("{grid: H4}")));
    EXPECT(nested->sameAs(*make("{grid: H4, order: nested}")));


    SECTION("fillGrib") {
        for (const auto* repres :
             {static_cast<const Representation*>(ring), static_cast<const Representation*>(nested)}) {
            grib_info info;
            repres->fillGrib(info);

            EXPECT_EQUAL(info.grid.grid_type, CODES_UTIL_GRID_SPEC_HEALPIX);
            EXPECT_EQUAL(info.grid.N, 4);
            EXPECT(same(info.grid.longitudeOfFirstGridPointInDegrees, 45.));
            EXPECT_EQUAL(extra_string(info, "orderingConvention"), repres->order());
        }
    }


    SECTION("fillMeshGen") {
        util::MeshGeneratorParameters a;
        ring->fillMeshGen(a);
        EXPECT_EQUAL(a.meshGenerator_, "structured");

        util::MeshGeneratorParameters b;
        nested->fillMeshGen(b);
        EXPECT_EQUAL(b.meshGenerator_, "delaunay");
    }


    SECTION("grid boxes, by point index (ring or nested)") {
        // the grid boxes contain their point
        for (const auto* repres :
             {static_cast<const Representation*>(ring), static_cast<const Representation*>(nested)}) {
            const auto boxes = repres->gridBoxes();
            ASSERT(boxes.size() == repres->numberOfPoints());

            for (std::unique_ptr<Iterator> it(repres->iterator()); it->next();) {
                const auto& box = boxes.at(it->index());
                const auto& p   = it->pointUnrotated();

                EXPECT(box.south() <= p.lat().value() && p.lat().value() <= box.north());
            }
        }
    }


    SECTION("by parametrisation (as GRIB)") {
        param::SimpleParametrisation param;
        param.set("gridType", "healpix");
        param.set("Nside", 4L);
        param.set("orderingConvention", "nested");
        param.set("longitudeOfFirstGridPointInDegrees", 45.);

        RepresentationHandle repres(RepresentationFactory::build(param));
        EXPECT(repres->sameAs(*nested));

        // the first longitude is 45 degrees
        param.set("longitudeOfFirstGridPointInDegrees", 0.);
        EXPECT_THROWS_AS(RepresentationFactory::build(param), eckit::AssertionFailed);
    }


    SECTION("by name") {
        struct test_t {
            std::string name;
            std::string gridname;
        };

        const std::vector<test_t> tests{{"H4", "H4"}, {"hR4", "H4"}, {"H4r", "H4"}, {"h4n", "HN4"}, {"HN4", "HN4"}};
        for (const auto& test : tests) {
            const auto& grid = key::grid::Grid::lookup(test.name);
            EXPECT_EQUAL(grid.gridname(), test.gridname);

            std::ostringstream str;
            str << grid;
            EXPECT(str.str().find(test.gridname) != std::string::npos);
            EXPECT_EQUAL(grid.gaussianNumber(), 8);

            RepresentationHandle repres(grid.representation());
            EXPECT_EQUAL(repres->uniqueName(), test.gridname);

            EXPECT_THROWS_AS(grid.representation(util::Rotation(-40., 22.)), eckit::NotImplemented);
        }
    }
}


CASE("HEALPix: interpolation to ring/nested orders, by name or gridSpec") {
    const std::string gridspec_ring   = R"({"grid":"H4"})";
    const std::string gridspec_nested = R"({"grid":"H4","order":"nested"})";

    struct test_t {
        std::string grid;
        std::string gridspec;
    };

    const std::vector<test_t> tests{
        {"H4", gridspec_ring},
        {"hR4", gridspec_ring},
        {"H4r", gridspec_ring},
        {"{grid: H4, order: ring}", gridspec_ring},
        {"h4n", gridspec_nested},
        {"HN4", gridspec_nested},
        {"{grid: H4, order: nested}", gridspec_nested},
    };


    SECTION("GRIB-based input") {
        const std::vector<std::string> inputs{
            "gridType=healpix,Nside=2,orderingConvention=ring.grib2",
            "gridType=healpix,Nside=2,orderingConvention=nested.grib2",
        };

        for (const auto& in : inputs) {
            for (const auto& out : tests) {
                api::MIRJob job;
                job.set("grid", out.grid);
                job.set("interpolation", "nn");

                std::unique_ptr<input::MIRInput> input(new input::GribFileInput(in));
                ASSERT(input->next());

                output::ArrayOutput result;
                job.execute(*input, result);

                EXPECT_EQUAL(result.gridspec(), out.gridspec);
            }
        }
    }


    SECTION("memory-based input") {
        for (const auto& in : tests) {
            param::GridSpecParametrisation param(in.grid);
            std::vector<double> values(param.grid().size(), 0.);

            EXPECT_EQUAL(param.spec().str(), in.gridspec);

            for (const auto& out : tests) {
                api::MIRJob job;
                job.set("grid", out.grid);
                job.set("interpolation", "nn");

                std::unique_ptr<input::MIRInput> input(new input::RawInput(values.data(), values.size(), param));
                ASSERT(input->next());

                output::ArrayOutput result;
                job.execute(*input, result);

                EXPECT_EQUAL(result.gridspec(), out.gridspec);
            }
        }
    }
}


CASE("HEALPix: ring/nested to ring/nested, values and GRIB encoding (MIR-647)") {
    struct test_t {
        std::string grid;
        std::string spec;
        std::string path;
        std::vector<double> expected;
    };

    const std::vector<test_t> tests{
        test_t{"H2",
               R"({"grid":"H2"})",
               "gridType=healpix,Nside=2,orderingConvention=ring.grib2",
               {
                   1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
                   25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48,
               }},
        test_t{"H2",
               R"({"grid":"H2"})",
               "gridType=healpix,Nside=2,orderingConvention=nested.grib2",
               {
                   4,  8,  12, 16, 3,  2,  7,  6,  11, 10, 15, 14, 20, 1,  24, 5,  28, 9,  32, 13, 18, 23, 22, 27,
                   26, 31, 30, 19, 17, 36, 21, 40, 25, 44, 29, 48, 35, 34, 39, 38, 43, 42, 47, 46, 33, 37, 41, 45,
               }},
        test_t{"H2n",
               R"({"grid":"H2","order":"nested"})",
               "gridType=healpix,Nside=2,orderingConvention=ring.grib2",
               {
                   14, 6,  5,  1,  16, 8,  7,  2,  18, 10, 9,  3,  20, 12, 11, 4,  29, 21, 28, 13, 31, 23, 22, 15,
                   33, 25, 24, 17, 35, 27, 26, 19, 45, 38, 37, 30, 46, 40, 39, 32, 47, 42, 41, 34, 48, 44, 43, 36,
               }},
        test_t{"H2n",
               R"({"grid":"H2","order":"nested"})",
               "gridType=healpix,Nside=2,orderingConvention=nested.grib2",
               {
                   1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
                   25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48,
               }},
    };

    for (const auto& test : tests) {
        SECTION(test.path + " to " + test.grid) {
            std::unique_ptr<input::MIRInput> input(new input::GribFileInput(test.path));
            ASSERT(input->next());

            std::vector<double> result;
            param::SimpleParametrisation meta;
            output::ResizableOutput output(result, meta);

            api::MIRJob job;
            job.set("grid", test.grid);
            job.set("interpolation", "nn");
            job.execute(*input, output);

            std::string grid;
            EXPECT(meta.get("grid", grid) && grid == test.spec);

            std::unique_ptr<const eckit::geo::Grid> r(eckit::geo::GridFactory::make_from_string(grid));
            std::unique_ptr<const eckit::geo::Grid> g(eckit::geo::GridFactory::make_from_string(test.grid));
            ASSERT(r && g);

            const auto first_lon = std::get<eckit::geo::PointLonLat>(r->first_point()).lon();

            EXPECT_EQUAL(g->spec_str(), test.spec);
            EXPECT_EQUAL(r->order(), g->order());
            EXPECT_EQUAL(r->type(), g->type());
            EXPECT_EQUAL(r->size(), g->size());
            EXPECT(eckit::types::is_approximately_equal(first_lon, 45.));
            EXPECT(*g == *r);

            EXPECT_EQUAL(result.size(), test.expected.size());
            for (size_t i = 0; i < result.size(); ++i) {
                EXPECT(eckit::types::is_approximately_equal(result[i], test.expected[i]));
            }

            // GRIB encoding
            std::vector<char> buffer(1024 * 1024);
            output::GribMemoryOutput output_grib(buffer.data(), buffer.size());
            job.execute(*input, output_grib);

            std::unique_ptr<codes_handle, decltype(&codes_handle_delete)> h(
                codes_handle_new_from_message(nullptr, buffer.data(), output_grib.length()), &codes_handle_delete);
            ASSERT(h);

            auto get_string = [&h](const char* key) {
                std::string value(256, 0);
                size_t len = value.size();
                ASSERT(CODES_SUCCESS == codes_get_string(h.get(), key, &value[0], &len) && len > 0);
                return std::string(value.data(), len - 1);  // remove trailing null
            };

            auto get_long = [&h](const char* key) {
                long value = 0;
                ASSERT(CODES_SUCCESS == codes_get_long(h.get(), key, &value));
                return value;
            };

            auto get_double = [&h](const char* key) {
                double value = 0;
                ASSERT(CODES_SUCCESS == codes_get_double(h.get(), key, &value));
                return value;
            };

            EXPECT_EQUAL(get_string("gridType"), "healpix");
            EXPECT_EQUAL(get_string("gridName"), "H2");
            EXPECT_EQUAL(get_string("orderingConvention"), r->order());
            EXPECT_EQUAL(get_long("Nside"), 2L);
            EXPECT_EQUAL(get_long("numberOfDataPoints"), static_cast<long>(r->size()));
            EXPECT(eckit::types::is_approximately_equal(get_double("longitudeOfFirstGridPointInDegrees"), first_lon));
        }
    }
}


}  // namespace mir::repres::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
