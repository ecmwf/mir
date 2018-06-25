/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


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


enum counting_mode_t { iterator, GRIB1, GRIB2 };


size_t numberOfPoints(const repres::Representation& repres, counting_mode_t counting, size_t gaussianNumber, size_t numberOfValues) {

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

    ASSERT(gaussianNumber > 0);
    std::string sample("reduced_gg_pl_" + std::to_string(gaussianNumber) + std::string(counting == GRIB1 ? "_grib1" : "_grib2"));

    grib_handle* handle = grib_handle_new_from_samples(nullptr, sample.c_str());
    HandleFree handle_detroy(handle);
    ASSERT(handle);


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

    info.packing.editionNumber = long(counting == GRIB1 ? 1 : 2);

    repres.fill(info);


    // Encode
    int flags = 0;
    grib_handle* result = grib_util_set_spec(handle, &info.grid, &info.packing, flags, &values[0], values.size(), &err);
    HandleFree result_destroy(result);
    GRIB_CALL(err);


    // Get number of values
    long n = 0;
    if (counting == iterator) {

        grib_iterator* iter = grib_iterator_new(result, 0, &err);
        if (err != GRIB_SUCCESS) {
            GRIB_CHECK(err, nullptr);
        }
        for (double lat, lon, value; grib_iterator_next(iter, &lat, &lon, &value); ++n) {
        }
        grib_iterator_delete(iter);

    } else {
        grib_get_long(result, "numberOfValues", &n);
    }
    ASSERT(n > 0);

    return size_t(n);
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

        RepresentationHandle cropped(grid->croppedRepresentation(bbox));

        size_t n = cropped->numberOfPoints();
        ASSERT(n > 0);
        EXPECT(n < nGlobal);
        log << "\tnumberOfPoints =\t" << n << std::endl;

        size_t numberOfPointsIterator = numberOfPoints(*cropped, counting_mode_t::iterator, gaussianNumber, n);
        EXPECT(numberOfPointsIterator == n);
        log << "\tecCodes iterator =\t" << numberOfPointsIterator << std::endl;

        size_t numberOfPoints1 = numberOfPoints(*cropped, counting_mode_t::GRIB1, gaussianNumber, n);
        EXPECT(numberOfPoints1 == n);
        log << "\tGRIB1 numberOfValues =\t" << n << std::endl;

        size_t numberOfPoints2 = numberOfPoints(*cropped, counting_mode_t::GRIB2, gaussianNumber, n);
        EXPECT(numberOfPoints2 == n);
        log << "\tGRIB2 numberOfValues =\t" << n << std::endl;
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

        RepresentationHandle cropped(grid->croppedRepresentation(bbox));

        size_t n = cropped->numberOfPoints();
        ASSERT(0 < n);
        EXPECT(n < nGlobal);
        log << "\tnumberOfPoints =\t" << n << std::endl;

        size_t numberOfPointsIterator = numberOfPoints(*cropped, counting_mode_t::iterator, gaussianNumber, n);
        EXPECT(numberOfPointsIterator == n);
        log << "\tecCodes iterator =\t" << numberOfPointsIterator << std::endl;

        size_t numberOfPoints1 = numberOfPoints(*cropped, counting_mode_t::GRIB1, gaussianNumber, n);
        EXPECT(numberOfPoints1 == n);
        log << "\tGRIB1 numberOfValues =\t" << n << std::endl;

        size_t numberOfPoints2 = numberOfPoints(*cropped, counting_mode_t::GRIB2, gaussianNumber, n);
        EXPECT(numberOfPoints2 == n);
        log << "\tGRIB2 numberOfValues =\t" << n << std::endl;
    }
}


}  // namespace unit
}  // namespace tests
}  // namespace mir


int main(int argc, char **argv) {
    return eckit::testing::run_tests(argc, argv, false);
}

