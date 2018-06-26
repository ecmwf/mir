/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include <vector>
#include "eckit/log/Log.h"
#include "eckit/memory/ScopedPtr.h"
#include "eckit/testing/Test.h"
#include "mir/config/LibMir.h"
#include "mir/repres/gauss/reduced/ReducedFromPL.h"
#include "mir/repres/gauss/reduced/ReducedOctahedral.h"
#include "mir/util/BoundingBox.h"
#include "mir/util/Grib.h"


namespace mir {
namespace tests {
namespace unit {


using repres::RepresentationHandle;
using util::BoundingBox;


static BoundingBox dummy;


void numberOfPoints(
        const repres::Representation& repres,
        size_t gaussianNumber,
        size_t numberOfValues,
        size_t& numberOfValues1,
        size_t& numberOfValues2,
        size_t& numberOfValuesIterator,
        BoundingBox& encodedBoundingBox = dummy) {

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

        grib_handle* result1 = grib_util_set_spec(handle, &info.grid, &info.packing, flags, &values[0], values.size(), &err);
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


size_t areaCropper(const repres::Representation& repres, BoundingBox& bbox) {

    Latitude n = 0;
    Latitude s = 0;
    Longitude e = 0;
    Longitude w = 0;

    size_t count = 0;
    bool first = true;

    // Iterator is "unrotated", because the cropping area
    // is expressed in before the rotation is applied
    eckit::ScopedPtr<repres::Iterator> iter(repres.iterator());
    while (iter->next()) {
        const repres::Iterator::point_ll_t& point = iter->pointUnrotated();
        if (bbox.contains(point.lat, point.lon)) {

            const Latitude& lat = point.lat;
            const Longitude lon = point.lon.normalise(bbox.west());

            if (first) {
                n = s = lat;
                e = w = lon;
                first = false;
            } else {
                if (n < lat) { n = lat; }
                if (s > lat) { s = lat; }
                if (e < lon) { e = lon; }
                if (w > lon) { w = lon; }
            }

            count++;
        }
    }

    // Don't support empty results
    ASSERT(count > 0);

    bbox = BoundingBox(n, w, s, e);
    return count;
}


CASE("test Gaussian iterator") {
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
}


CASE("test area point count (O640)") {
    auto& log = eckit::Log::info();

    const size_t gaussianNumber(640);
    RepresentationHandle grid(new repres::gauss::reduced::ReducedOctahedral(gaussianNumber));

    log << "Test " << *grid << std::endl;
    size_t nGlobal = grid->numberOfPoints();
    ASSERT(nGlobal > 0);
    log << "\tnumberOfPoints =\t" << nGlobal << std::endl;

    for (auto& bbox : {
         BoundingBox{  51.941,    7.005,   43.084,   27.693 },
         BoundingBox{  51.9406,   7.00599, 43.0847,  27.6923 },
         BoundingBox{  57.9852, 230,       25.0918, 300 },
         BoundingBox{  11.8782, 279,      -49.9727, 325 },
         BoundingBox{ -25.0918, 135,      -46.8801, 179 },
         BoundingBox{  43.9281,  91,       21.0152, 143 },
         BoundingBox{  59.9531,  23,       35.0722,  80 },
        }) {
        log << "Test " << *grid << " with " << bbox << "..." << std::endl;

        // crop "manually", to get the smallest possible bounding box
        BoundingBox small = bbox;
        size_t n = areaCropper(*grid, small);
        ASSERT(0 < n);
        EXPECT(n < nGlobal);
        log << "\tnumberOfPoints =\t" << n << std::endl;

        RepresentationHandle cropped(grid->croppedRepresentation(bbox));
        EXPECT(n == cropped->numberOfPoints());  // (heavy!)

        // check point counts
        ASSERT(n > 0);
        EXPECT(n < nGlobal);
        log << "\tnumberOfPoints =\t" << n << std::endl;

        size_t numberOfValues1 = 0;
        size_t numberOfValues2 = 0;
        size_t numberOfValuesIterator = 0;
        numberOfPoints(*cropped, gaussianNumber, n, numberOfValues1, numberOfValues2, numberOfValuesIterator);
        log <<   "\tGRIB1 numberOfValues =\t" << numberOfValues1
            << "\n\tGRIB2 numberOfValues =\t" << numberOfValues2
            << "\n\tecCodes iterator =\t" << numberOfValuesIterator
            << std::endl;
        EXPECT(numberOfValues1 == n);
        EXPECT(numberOfValues2 == n);
        EXPECT(numberOfValuesIterator == n);
    }
}


CASE("test area point count (O1280)") {
    auto& log = eckit::Log::info();

    const size_t gaussianNumber(1280);
    RepresentationHandle grid(new repres::gauss::reduced::ReducedOctahedral(gaussianNumber));

    log << "Test " << *grid << std::endl;
    size_t nGlobal = grid->numberOfPoints();
    ASSERT(nGlobal > 0);
    log << "\tnumberOfPoints =\t" << nGlobal << std::endl;

    for (auto& bbox : {
         BoundingBox{  90., 0., -90., 359.929 },
        }) {
        log << "Test " << *grid << " with " << bbox << "..." << std::endl;

        // crop "manually", to get the smallest possible bounding box
        BoundingBox small = bbox;
        size_t n = areaCropper(*grid, small);
        ASSERT(0 < n);
        EXPECT(n < nGlobal);
        log << "\tnumberOfPoints =\t" << n << std::endl;

        RepresentationHandle cropped(grid->croppedRepresentation(bbox));
        EXPECT(n == cropped->numberOfPoints());  // (heavy!)

        ASSERT(0 < n);
        EXPECT(n < nGlobal);
        log << "\tnumberOfPoints =\t" << n << std::endl;

        size_t numberOfValues1 = 0;
        size_t numberOfValues2 = 0;
        size_t numberOfValuesIterator = 0;
        numberOfPoints(*cropped, gaussianNumber, n, numberOfValues1, numberOfValues2, numberOfValuesIterator);
        log <<   "\tGRIB1 numberOfValues =\t" << numberOfValues1
            << "\n\tGRIB2 numberOfValues =\t" << numberOfValues2
            << "\n\tecCodes iterator =\t" << numberOfValuesIterator
            << std::endl;
        EXPECT(numberOfValues1 == n);
        EXPECT(numberOfValues2 == n);
        EXPECT(numberOfValuesIterator == n);
    }
}


CASE("ECC-445 (O1280)") {
    auto& log = eckit::Log::info();

    const size_t gaussianNumber(1280);
    RepresentationHandle grid(new repres::gauss::reduced::ReducedOctahedral(gaussianNumber));

    log << "Test " << *grid << std::endl;
    size_t nGlobal = grid->numberOfPoints();
    ASSERT(nGlobal > 0);
    log << "\tnumberOfPoints =\t" << nGlobal << std::endl;

    struct test_t {
        size_t numberOfPoints;
        BoundingBox bbox;
    };

    for (auto& test : {
         test_t{  12369, {  37.6025, -114.891,  27.7626, -105.188 }},
         test_t{     19, {  27.9,     253,      27.8,     254 }},
         test_t{ 124577, { -10.017,   -85,     -38.981,   -56 }},
         test_t{ 124209, { -10.0176,  275,     -38.9807,  304 }},
         test_t{  12274, {  37.5747,  245.109,  27.8032,  254.812 }},
         test_t{  12373, {  37.575,  -114.892,  27.803,  -105.187 }},
         test_t{  12373, {  37.6025, -114.8915, 27.7626, -105.1875 }},
         test_t{ 124143, { -10, -85,  -39,     -56.1 }},
        }) {
        log << "Test " << *grid << " with " << test.bbox << "..." << std::endl;

        // crop "manually", to get the smallest possible bounding box
        BoundingBox small = test.bbox;
        size_t n = areaCropper(*grid, small);
        ASSERT(0 < n);
        EXPECT(n < nGlobal);
        log << "\tnumberOfPoints =\t" << n << std::endl;

        RepresentationHandle cropped(grid->croppedRepresentation(test.bbox));
        EXPECT(n == cropped->numberOfPoints());  // (heavy!)

        size_t numberOfValues1 = 0;
        size_t numberOfValues2 = 0;
        size_t numberOfValuesIterator = 0;
        BoundingBox encoded;
        numberOfPoints(*cropped, gaussianNumber, n, numberOfValues1, numberOfValues2, numberOfValuesIterator, encoded);

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


}  // namespace unit
}  // namespace tests
}  // namespace mir


int main(int argc, char **argv) {
    return eckit::testing::run_tests(argc, argv, false);
}

