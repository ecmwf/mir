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

#include "eckit/config/Resource.h"
#include "eckit/geo/Grid.h"
#include "eckit/testing/Test.h"
#include "eckit/types/FloatCompare.h"

#include "mir/api/MIRJob.h"
#include "mir/input/GribFileInput.h"
#include "mir/input/MIRInput.h"
#include "mir/output/GribMemoryOutput.h"
#include "mir/output/ResizableOutput.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/repres/Representation.h"
#include "mir/util/Grib.h"


namespace mir::tests::unit {


CASE("HEALPix order:nested/ring to order:nested/ring") {
    struct test_type {
        const std::string grid;
        const std::string spec;
        const std::string path;
        const std::vector<double> expected;
    };

    for (const auto& test : {
             test_type{
                 "H2",                                                      //
                 R"({"grid":"H2"})",                                        //
                 "gridType=healpix,Nside=2,orderingConvention=ring.grib2",  //
                 {
                     1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
                     25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48,
                 }},
             test_type{
                 "H2",                                                        //
                 R"({"grid":"H2"})",                                          //
                 "gridType=healpix,Nside=2,orderingConvention=nested.grib2",  //
                 {
                     4,  8,  12, 16, 3,  2,  7,  6,  11, 10, 15, 14, 20, 1,  24, 5,  28, 9,  32, 13, 18, 23, 22, 27,
                     26, 31, 30, 19, 17, 36, 21, 40, 25, 44, 29, 48, 35, 34, 39, 38, 43, 42, 47, 46, 33, 37, 41, 45,
                 }},
             test_type{
                 "H2n",                                                     //
                 R"({"grid":"H2","order":"nested"})",                       //
                 "gridType=healpix,Nside=2,orderingConvention=ring.grib2",  //
                 {
                     14, 6,  5,  1,  16, 8,  7,  2,  18, 10, 9,  3,  20, 12, 11, 4,  29, 21, 28, 13, 31, 23, 22, 15,
                     33, 25, 24, 17, 35, 27, 26, 19, 45, 38, 37, 30, 46, 40, 39, 32, 47, 42, 41, 34, 48, 44, 43, 36,
                 }},
             test_type{
                 "H2n",                                                       //
                 R"({"grid":"H2","order":"nested"})",                         //
                 "gridType=healpix,Nside=2,orderingConvention=nested.grib2",  //
                 {
                     1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
                     25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48,
                 }},
         }) {
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
        ASSERT(r);

        std::unique_ptr<const eckit::geo::Grid> g(eckit::geo::GridFactory::make_from_string(test.grid));
        ASSERT(g);

        auto first_lon = std::get<eckit::geo::PointLonLat>(r->first_point()).lon();

        EXPECT(g->spec_str() == test.spec);
        EXPECT(r->order() == g->order());
        EXPECT(r->type() == g->type());
        EXPECT(r->size() == g->size());
        EXPECT(eckit::types::is_approximately_equal(first_lon, 45.));
        EXPECT(*g == *r);

        EXPECT(result.size() == test.expected.size());
        for (size_t i = 0; i < result.size(); ++i) {
            EXPECT(eckit::types::is_approximately_equal(result[i], test.expected[i]));
        }


        static size_t buffer_size = eckit::Resource<size_t>("$MIR_GRIB_OUTPUT_BUFFER_SIZE", 64 * 1024 * 1024);
        std::unique_ptr<char[]> buffer(new char[buffer_size]);

        output::GribMemoryOutput output_grib(buffer.get(), buffer_size);
        job.execute(*input, output_grib);


        std::unique_ptr<codes_handle, decltype(&codes_handle_delete)> h(
            codes_handle_new_from_message(nullptr, buffer.get(), buffer_size), &codes_handle_delete);

        auto get_string = [](const auto& h, const char* key) {
            std::string value(256, 0);
            size_t len = value.size();
            ASSERT(CODES_SUCCESS == codes_get_string(h.get(), key, value.data(), &len) && len > 0);
            return std::string(value.data(), len - 1);  // remove trailing null
        };

        auto get_long = [](const auto& h, const char* key) {
            long value = 0;
            ASSERT(CODES_SUCCESS == codes_get_long(h.get(), key, &value));
            return value;
        };

        auto get_double = [](const auto& h, const char* key) {
            double value = 0;
            ASSERT(CODES_SUCCESS == codes_get_double(h.get(), key, &value));
            return value;
        };

        EXPECT(get_string(h, "gridType") == "healpix");
        EXPECT(get_string(h, "gridName") == "H2");
        EXPECT(get_string(h, "orderingConvention") == r->order());
        EXPECT(get_long(h, "Nside") == 2L);
        EXPECT(get_long(h, "numberOfDataPoints") == static_cast<long>(r->size()));
        EXPECT(eckit::types::is_approximately_equal(get_double(h, "longitudeOfFirstGridPointInDegrees"), first_lon));
    }
}


}  // namespace mir::tests::unit


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
