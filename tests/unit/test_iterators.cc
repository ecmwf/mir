/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include <string>
#include <vector>
#include "eckit/log/Log.h"
#include "eckit/memory/ScopedPtr.h"
#include "eckit/testing/Test.h"
#include "mir/action/misc/AreaCropper.h"
#include "mir/config/LibMir.h"
#include "mir/namedgrids/NamedGrid.h"
#include "mir/repres/gauss/reduced/ReducedFromPL.h"
#include "mir/repres/gauss/reduced/ReducedOctahedral.h"
#include "mir/util/BoundingBox.h"
#include "mir/util/Grib.h"


namespace mir {
namespace tests {
namespace unit {


using repres::RepresentationHandle;
using util::BoundingBox;


void numberOfPoints(
        const repres::Representation& repres,
        size_t gaussianNumber,
        size_t numberOfValues,
        size_t& numberOfValues1,
        size_t& numberOfValues2,
        size_t& numberOfValuesIterator,
        BoundingBox& encodedBoundingBox) {

    // Make sure handles are deleted even in case of exception
    class HandleFree {
        grib_handle *h_;
    public:
        HandleFree(grib_handle *h): h_(h) {}
        HandleFree(const HandleFree&) = delete;
        void operator=(const HandleFree&) = delete;
        ~HandleFree() {
            if (h_) {
                grib_handle_delete(h_);
            }
        }
    };

    // Fill info to pass to grib_util_set_spec
    // (with dummy paramId "Indicates a missing value" and values)
    int err = 0;
    grib_info info = {{0,}};

    ASSERT(numberOfValues > 0);
    std::vector<double> values(numberOfValues, 0.);

    auto j = info.packing.extra_settings_count++;
    info.packing.extra_settings[j].name = "paramId";
    info.packing.extra_settings[j].type = GRIB_TYPE_LONG;
    info.packing.extra_settings[j].long_value = 129255;

    repres.fill(info);


    // Encode
    int flags = 0;


    // Get encoded numberOfValues in GRIB1
    {
        ASSERT(gaussianNumber > 0);
        std::string sample("reduced_gg_pl_" + std::to_string(gaussianNumber) + "_grib1");

        grib_handle* handle = grib_handle_new_from_samples(nullptr, sample.c_str());
        HandleFree handle_detroy(handle);
        ASSERT(handle);

        info.packing.editionNumber = 1L;

        grib_handle* result1 = grib_util_set_spec(handle, &info.grid, &info.packing, flags, values.data(), values.size(), &err);
        HandleFree result1_destroy(result1);
        GRIB_CALL(err);

        long n = 0;
        grib_get_long(result1, "numberOfValues", &n);
        ASSERT(n > 0);
        numberOfValues1 = size_t(n);
    }


    // Get encoded numberOfValues in GRIB2, iterator, and bounding box
    {
        ASSERT(gaussianNumber > 0);
        std::string sample("reduced_gg_pl_" + std::to_string(gaussianNumber) + "_grib2");

        grib_handle* handle = grib_handle_new_from_samples(nullptr, sample.c_str());
        HandleFree handle_detroy(handle);
        ASSERT(handle);

        info.packing.editionNumber = 2L;

        grib_handle* result2 = grib_util_set_spec(handle, &info.grid, &info.packing, flags, &values[0], values.size(), &err);
        HandleFree result2_destroy(result2);
        GRIB_CALL(err);

        long n = 0;
        grib_get_long(result2, "numberOfValues", &n);
        ASSERT(n > 0);
        numberOfValues2 = size_t(n);

        grib_iterator* iter = grib_iterator_new(result2, 0, &err);
        if (err != GRIB_SUCCESS) {
            GRIB_CHECK(err, nullptr);
        }

        n = 0;
        for (double lat, lon, value; grib_iterator_next(iter, &lat, &lon, &value); ++n) {
        }
        ASSERT(n > 0);

        grib_iterator_delete(iter);
        numberOfValuesIterator = size_t(n);

        // Get encoded bounding box
        double box[4];
        grib_get_double(result2, "latitudeOfFirstGridPointInDegrees",  &box[0]);
        grib_get_double(result2, "longitudeOfFirstGridPointInDegrees", &box[1]);
        grib_get_double(result2, "latitudeOfLastGridPointInDegrees",   &box[2]);
        grib_get_double(result2, "longitudeOfLastGridPointInDegrees",  &box[3]);

        encodedBoundingBox = BoundingBox(box[0], box[1], box[2], box[3]);
    }
}


CASE("test Gaussian iterator") {
#if 0
    auto& out = eckit::Log::info();
    auto& debug = eckit::Log::debug<LibMir>();

    const std::vector<long> pl{20, 24, 28, 28, 24, 20};
    auto& latitudes = repres::Gaussian::latitudes(3);

    repres::RepresentationHandle rep(new repres::gauss::reduced::ReducedFromPL(3, pl));
    eckit::ScopedPtr<repres::Iterator> iter(rep->iterator());

    while (iter->next()) {
        auto& p = iter->pointUnrotated();
        out << p << std::endl;
    }
#endif
}


CASE("test area point count") {

    auto& log = eckit::Log::info();


    struct test_t {
        test_t(std::string&& _gridname, BoundingBox&& _bbox, size_t _numberOfPoints, size_t _Ni = 0, size_t _Nj =0) :
            gridname(_gridname),
            bbox(_bbox),
            numberOfPoints(_numberOfPoints),
            Ni(_Ni),
            Nj(_Nj) {
        }
        std::string gridname;
        BoundingBox bbox;
        size_t numberOfPoints;
        size_t Ni;
        size_t Nj;
    };


    for (auto& test : std::vector<test_t>{

         // pgen
         { "O640",  BoundingBox{  51.941,    7.005,   43.084,    27.693 },    4512 },
         { "O640",  BoundingBox{  51.9406,   7.00599, 43.0847,   27.6923 },   4443 },
         { "O640",  BoundingBox{  57.9852, 230,       25.0918,  300 },       63479 },
         { "O640",  BoundingBox{  11.8782, 279,      -49.9727,  325 },      111068 },
         { "O640",  BoundingBox{ -25.0918, 135,      -46.8801,  179 },       29294 },
         { "O640",  BoundingBox{  43.9281,  91,       21.0152,  143 },       38990 },
         { "O640",  BoundingBox{  59.9531,  23,       35.0722,   80 },       34426 },

         // "almost" global
         test_t{ "O1280", BoundingBox{  90., 0., -90., 359.929 }, 6599646 },

         // ECC-445
//         { "O1280", BoundingBox{  37.6025, -114.891,  27.7626, -105.188 },   12369 },
//         { "O1280", BoundingBox{  27.9,     253,      27.8,     254 },          19 },
         { "O1280", BoundingBox{ -10.017,   -85,     -38.981,   -56 },      124577 },
         { "O1280", BoundingBox{ -10.0176,  275,     -38.9807,  304 },      124209 },
//         { "O1280", BoundingBox{  37.5747,  245.109,  27.8032,  254.812 },   12274 },
//         { "O1280", BoundingBox{  37.575,  -114.892,  27.803,  -105.187 },   12373 },
//         { "O1280", BoundingBox{  37.6025, -114.8915, 27.7626, -105.1875 },  12373 },
         { "O1280", BoundingBox{ -10,       -85,     -39,       -56.1 },    124143 },
         { "F160",  BoundingBox{  40,        50,     -50,       169.532 },   34080, 213, 160 },
         { "F160",  BoundingBox{  71.8,     -10.66,   34.56,     32.6 },      5016,  76,  66 },
         { "F320",  BoundingBox{  70.9,     -40.987,  19.73,     40 },       52416, 288, 182 },
         { "F640",  BoundingBox{  70.9,     -40.987,  19.73,     40 },      209664, 576, 364 },

        }) {

        auto& ng = namedgrids::NamedGrid::lookup(test.gridname);
        RepresentationHandle grid(ng.representation());

        log << "Test " << *grid << " with " << test.bbox << "..." << std::endl;


        // Global representation
        size_t nGlobal = grid->numberOfPoints();
        ASSERT(0 < nGlobal);
        log << "\tnumberOfPoints =\t" << nGlobal << " (global)" << std::endl;



        // Crop "manually", to get the smallest possible bounding box
        std::vector<size_t> mapping;
        BoundingBox small(test.bbox);
        action::AreaCropper::crop(*grid, small, mapping);

        size_t nCropped = mapping.size();
        ASSERT(0 < nCropped);
        log << "\tnumberOfPoints =\t" << nCropped << " (manual crop)" << std::endl;

        EXPECT(nCropped < nGlobal);
        EXPECT(test.bbox.contains(small));


        // Cropped representation
        RepresentationHandle cropped(grid->croppedRepresentation(test.bbox));
        size_t n = cropped->numberOfPoints();
        ASSERT(0 < n);
        log << "\tnumberOfPoints =\t" << n << " (representation crop)" << std::endl;

        EXPECT(n < nGlobal);
        EXPECT(n == nCropped);

        size_t numberOfValues1 = 0;
        size_t numberOfValues2 = 0;
        size_t numberOfValuesIterator = 0;
        BoundingBox encoded;
        numberOfPoints(*cropped, ng.gaussianNumber(), n, numberOfValues1, numberOfValues2, numberOfValuesIterator, encoded);

        log <<   "\tGRIB1 numberOfValues =\t" << numberOfValues1
            << "\n\tGRIB2 numberOfValues =\t" << numberOfValues2
            << "\n\tecCodes iterator =\t" << numberOfValuesIterator
            << std::endl;

        EXPECT(numberOfValues1 == n);
        EXPECT(numberOfValues2 == n);
        EXPECT(numberOfValuesIterator == n);

        log <<   "\tcrop =\t" << test.bbox
            << "\n\tbbox =\t" << small << " (contained by crop)"
            << "\n\tencoded = " << encoded << " (expanded bbox)"
            << std::endl;

        EXPECT(test.bbox.contains(small));
        EXPECT(encoded.contains(small));
    }
}




#if 0
--area=58.5/-6.1/36/20.7 --grid=0.1/0.1
# grib_get
numberOfValues=60794
latitudeOfFirstGridPointInDegrees=58.5
longitudeOfFirstGridPointInDegrees=-6.1
latitudeOfLastGridPointInDegrees=36
longitudeOfLastGridPointInDegrees=20.7
Ni=269
Nj=226
#endif



}  // namespace unit
}  // namespace tests
}  // namespace mir


int main(int argc, char **argv) {
    return eckit::testing::run_tests(argc, argv, false);
}

