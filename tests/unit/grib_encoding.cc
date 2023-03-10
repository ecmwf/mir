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

#include "eckit/testing/Test.h"

#include "mir/action/area/AreaCropper.h"
#include "mir/api/mir_config.h"
#include "mir/data/MIRField.h"
#include "mir/input/GribMemoryInput.h"
#include "mir/key/grid/Grid.h"
#include "mir/repres/Iterator.h"
#include "mir/repres/Representation.h"
#include "mir/repres/latlon/RegularLL.h"
#include "mir/util/BoundingBox.h"
#include "mir/util/Grib.h"
#include "mir/util/Log.h"
#include "mir/util/Mutex.h"


namespace mir::tests::unit {

using input::MIRInput;
using repres::RepresentationHandle;
using util::BoundingBox;

static util::recursive_mutex local_mutex;
static std::vector<bool> _yes_no{true, false};
static std::vector<long> _one_two{1, 2};

namespace {


class EncodeTest {

    RepresentationHandle representation_;
    grib_handle* grib1Handle_;
    grib_handle* grib2Handle_;
    MIRInput* grib1Input_;
    MIRInput* grib2Input_;

    virtual std::string gribSample(long edition) const = 0;

protected:
    grib_handle* gribHandle(long edition) {
        util::lock_guard<util::recursive_mutex> lock(local_mutex);

        ASSERT(edition == 1 || edition == 2);
        grib_handle*& handle(edition == 1 ? grib1Handle_ : grib2Handle_);

        if (handle == nullptr) {

            grib_info info;

            // paramId "Indicates a missing value"
            auto j                                    = info.packing.extra_settings_count++;
            info.packing.extra_settings[j].name       = "paramId";
            info.packing.extra_settings[j].type       = CODES_TYPE_LONG;
            info.packing.extra_settings[j].long_value = 129255;

            info.packing.editionNumber = edition;

            info.grid.missingValue = 2.;

            representation_->fillGrib(info);

            size_t n = numberOfValues();
            ASSERT(n);
            std::vector<double> values(n, 0.);
            values[0] = 1.;

            // Make sure handles are deleted even in case of exception
            auto sample = grib_handle_new_from_samples(nullptr, gribSample(edition).c_str());
            HandleDeleter sample_detroy(sample);

            int err   = 0;
            int flags = 0;
            handle =
                codes_grib_util_set_spec(sample, &info.grid, &info.packing, flags, values.data(), values.size(), &err);
            GRIB_CALL(err);

            //            grib_write_message(handle,("error.grib" + std::to_string(edition)).c_str(),"w");
        }

        ASSERT(handle != nullptr);
        return handle;
    }

    const MIRInput& gribInput(long edition) {
        util::lock_guard<util::recursive_mutex> lock(local_mutex);

        ASSERT(edition == 1 || edition == 2);
        MIRInput*& input(edition == 1 ? grib1Input_ : grib2Input_);

        if (input == nullptr) {

            const void* message;
            size_t length;
            GRIB_CALL(grib_get_message(gribHandle(edition), &message, &length));

            input = new input::GribMemoryInput(message, length);
        }

        ASSERT(input != nullptr);
        return *input;
    }

public:
    EncodeTest(const repres::Representation* rep) :
        representation_(rep),
        grib1Handle_(nullptr),
        grib2Handle_(nullptr),
        grib1Input_(nullptr),
        grib2Input_(nullptr) {}

    virtual ~EncodeTest() {
        util::lock_guard<util::recursive_mutex> lock(local_mutex);

        grib_handle_delete(grib1Handle_);
        grib_handle_delete(grib2Handle_);
        delete grib1Input_;
        delete grib2Input_;
    }

    EncodeTest(const EncodeTest&)            = delete;
    EncodeTest(EncodeTest&&)                 = delete;
    EncodeTest& operator=(const EncodeTest&) = delete;
    EncodeTest& operator=(EncodeTest&&)      = delete;

    virtual size_t numberOfValues() const = 0;

    size_t numberOfValuesEncodedInGrib(long edition) {
        util::lock_guard<util::recursive_mutex> lock(local_mutex);

        long n = 0;
        grib_get_long(gribHandle(edition), "numberOfValues", &n);

        ASSERT(n > 0);
        return size_t(n);
    }

    size_t numberOfValuesFromGribIterator(long edition) {
        util::lock_guard<util::recursive_mutex> lock(local_mutex);

        int err   = 0;
        auto iter = grib_iterator_new(gribHandle(edition), 0, &err);
        if (err != CODES_SUCCESS) {
            CODES_CHECK(err, nullptr);
        }

        long n = 0;
        for (double lat, lon, value; grib_iterator_next(iter, &lat, &lon, &value) != 0; ++n) {
        }

        grib_iterator_delete(iter);

        ASSERT(n > 0);
        return size_t(n);
    }

    bool compareCoordinates(long edition, double toleranceLat, double toleranceLon) {
        util::lock_guard<util::recursive_mutex> lock(local_mutex);

        std::unique_ptr<repres::Iterator> iter_m(representation_->iterator());

        int err     = 0;
        auto iter_g = grib_iterator_new(gribHandle(edition), 0, &err);
        if (err != CODES_SUCCESS) {
            CODES_CHECK(err, nullptr);
        }

        long n = 0;
        for (double lat, lon, value; grib_iterator_next(iter_g, &lat, &lon, &value) != 0; ++n) {
            ASSERT(iter_m->next());

            double dlat = mir::Latitude(iter_m->pointRotated()[0]).distance(lat).value();
            double dlon = mir::LongitudeDouble(iter_m->pointRotated()[1]).distance(lon).value();

            if (dlat > toleranceLat || dlon > toleranceLon) {
                return false;
            }
        }

        grib_iterator_delete(iter_g);

        ASSERT(!iter_m->next());
        ASSERT(n > 0);

        return true;
    }

#if 0
    size_t numberOfValuesFromGribInput(long edition) {
        RepresentationHandle rep = gribInput(edition).field().representation();
        return rep->numberOfPoints();
    }
#endif

#if 0
    BoundingBox boundingBoxEncodedInGrib(long edition) {
        util::lock_guard<util::recursive_mutex> lock(local_mutex);

        grib_handle* handle = gribHandle(edition);
        ASSERT(handle);

        double box[4];
        grib_get_double(handle, "latitudeOfFirstGridPointInDegrees", &box[0]);
        grib_get_double(handle, "longitudeOfFirstGridPointInDegrees", &box[1]);
        grib_get_double(handle, "latitudeOfLastGridPointInDegrees", &box[2]);
        grib_get_double(handle, "longitudeOfLastGridPointInDegrees", &box[3]);

        return BoundingBox(box[0], box[1], box[2], box[3]);
    }
#endif

    BoundingBox boundingBoxFromGribInput(long edition) {
        RepresentationHandle rep = gribInput(edition).field().representation();
        return rep->boundingBox();
    }
};


class EncodeReduced : public EncodeTest {
    size_t numberOfValues_;
    size_t numberOfValues() const override { return numberOfValues_; }

public:
    EncodeReduced(const repres::Representation* rep, size_t numberOfValues) :
        EncodeTest(rep), numberOfValues_(numberOfValues) {
        ASSERT(numberOfValues_);
    }
};


class EncodeReducedGaussianGrid final : public EncodeReduced {
    size_t gaussianNumber_;
    std::string gribSample(long edition) const override {
        return std::string("reduced_gg_pl_" + std::to_string(gaussianNumber_) + "_grib" + std::to_string(edition));
    }

public:
    EncodeReducedGaussianGrid(const repres::Representation* rep, size_t numberOfValues, size_t gaussianNumber) :
        EncodeReduced(rep, numberOfValues), gaussianNumber_(gaussianNumber) {
        ASSERT(gaussianNumber_);
    }
};


class EncodeRegular : public EncodeTest {
    size_t Ni_;
    size_t Nj_;

    size_t numberOfValues() const override { return Ni_ * Nj_; }

public:
    EncodeRegular(const repres::Representation* rep, size_t Ni, size_t Nj) : EncodeTest(rep), Ni_(Ni), Nj_(Nj) {
        ASSERT(Ni_);
        ASSERT(Nj_);
    }

    ~EncodeRegular() override = default;

    EncodeRegular(const EncodeRegular&)            = delete;
    EncodeRegular(EncodeRegular&&)                 = delete;
    EncodeRegular& operator=(const EncodeRegular&) = delete;
    EncodeRegular& operator=(EncodeRegular&&)      = delete;

    size_t NiEncodedInGrib(long edition) {
        util::lock_guard<util::recursive_mutex> lock(local_mutex);

        long n = 0;
        grib_get_long(gribHandle(edition), "Ni", &n);

        ASSERT(n > 0);
        return size_t(n);
    }

    size_t NjEncodedInGrib(long edition) {
        util::lock_guard<util::recursive_mutex> lock(local_mutex);

        long n = 0;
        grib_get_long(gribHandle(edition), "Nj", &n);

        ASSERT(n > 0);
        return size_t(n);
    }
};


class EncodeRegularGaussianGrid final : public EncodeRegular {
    std::string gribSample(long edition) const override {
        return std::string("regular_gg_pl_grib" + std::to_string(edition));
    }

public:
    EncodeRegularGaussianGrid(const repres::Representation* rep, size_t Ni, size_t Nj, size_t gaussianNumber) :
        EncodeRegular(rep, Ni, Nj) {
        ASSERT(gaussianNumber);
    }
};


class EncodeRegularLatLonGrid final : public EncodeRegular {
    std::string gribSample(long edition) const override {
        return std::string("regular_ll_pl_grib" + std::to_string(edition));
    }

public:
    using EncodeRegular::EncodeRegular;
};


}  // namespace


CASE("GRIB1/GRIB2 encoding of sub-area of reduced Gaussian grids") {
    auto& log = Log::info();

    struct test_t {
        std::string grid;
        BoundingBox bbox;
        size_t count;
    };

    std::vector<test_t> _test {
        // pgen
        test_t{"O640", {51.941, 7.005, 43.084, 27.693}, 4512},
            test_t{"O640", {51.9406, 7.00599, 43.0847, 27.6923}, 4443},
            test_t{"O640", {57.9852, 230, 25.0918, 300}, 63479}, test_t{"O640", {11.8782, 279, -49.9727, 325}, 111068},
            test_t{"O640", {-25.0918, 135, -46.8801, 179}, 29294}, test_t{"O640", {43.9281, 91, 21.0152, 143}, 38990},
            test_t{"O640", {59.9531, 23, 35.0722, 80}, 34426},

            // ECC-445
            test_t{"O1280", {-10.017, -85, -38.981, -56}, 124577},
            test_t{"O1280", {-10.017, 275, -38.981, 304}, 124577}, test_t{"O1280", {-10, -85, -39, -56.1}, 124143},

#if mir_HAVE_ATLAS
            // ECC-576
            test_t{"N256", {90, 0, -90, 359.6489}, 348528}, test_t{"N256", {90, 0, -90, 359.9}, 348528},
            test_t{"N640", {90, 0, -90, 359.9}, 2140702}, test_t{"N640", {90, 0, -90, 359.99}, 2140702},
            test_t{"N640", {90, -180, -90, 179.99}, 2140702}, test_t{"O640", {90, 0, -90, 359.999}, 1661440},
#endif

#if 0
             // MIR-390: resolution triggers these
             // FIXME: issues decoding with MIR, because West/East converted to fraction go "inwards"
             test_t{"O1280", {37.6025, -114.891, 27.7626, -105.188}, 12369},
             test_t{"O1280", {27.9, 253, 27.8, 254}, 19},
             test_t{"O1280", {37.5747, 245.109, 27.8032, 254.812}, 12274},
             test_t{"O1280", {37.575, -114.892, 27.803, -105.187}, 12373},
             test_t{"O1280", {37.6025, -114.8915, 27.7626, -105.1875}, 12373},
#endif

        // "almost global"
        // NOTE: this cannot be supported because:
        // * Lo2=359929 is encoded for GRIB1 O1280 global fields
        // * Lo2=359930 should be the coorrect value (the real value is 360 - 15/214 ~= 359.929907)
        // so GRIB1 O1280 fields are actually not correctly encoded
        //         test_t{ "O1280", {  90., 0., -90., 359.929 }, 6599646 },
    };

    for (auto& test : _test) {

        const auto& ng = key::grid::Grid::lookup(test.grid);
        const RepresentationHandle rep(ng.representation());

        log << "Test " << *rep << " with " << test.bbox << "..." << std::endl;

        // Crop to get the smallest possible bounding box
        std::vector<size_t> mapping;
        BoundingBox small(test.bbox);
        action::AreaCropper::crop(*rep, small, mapping);

        size_t n = mapping.size();
        ASSERT(0 < n);
        log << "\tnumberOfPoints = " << n << " (crop)" << std::endl;

        EXPECT(test.count == n);
        EXPECT(test.bbox.contains(small));

        // GRIB1/GRIB2 encoding
        EncodeReducedGaussianGrid enc(rep->croppedRepresentation(test.bbox), test.count, ng.gaussianNumber());
        EncodeTest& encode = enc;

        for (long edition : _one_two) {

            const size_t numberOfValues = encode.numberOfValuesEncodedInGrib(edition);
            log << "\tGRIB" << edition << ": numberOfValues = " << numberOfValues << " (key)" << std::endl;
            EXPECT(numberOfValues == n);

            const size_t numberOfValuesIt = encode.numberOfValuesFromGribIterator(edition);
            log << "\tGRIB" << edition << ": numberOfValues = " << numberOfValuesIt << " (iterator)" << std::endl;
            EXPECT(numberOfValuesIt == n);

            const BoundingBox bbox = encode.boundingBoxFromGribInput(edition);
            log << "\tGRIB" << edition << ": " << bbox << std::endl;
            EXPECT(bbox.contains(small));

            // Compare mir/eccodes iterators coordinates with a better precision
            if (test.grid != "O1280") {  // FIXME: ECC-747
                double tol = 1.e-3;
                log << "\tGRIB" << edition << ": |Δ(lat,lon)| <= (" << tol << ", " << tol << ")" << std::endl;
                EXPECT(encode.compareCoordinates(edition, tol, tol));
            }
        }
    }
}


CASE("GRIB1/GRIB2 encoding of sub-area of regular Gaussian grids") {
    auto& log = Log::info();

    struct test_t {
        std::string grid;
        BoundingBox bbox;
        size_t Ni;
        size_t Nj;
    };

    // ECC-445
    std::vector<test_t> _test{
        test_t{"F160", {71.8, -10.66, 34.56, 32.6}, 76, 66},
        test_t{"F160", {40, 50, -50, 169.532}, 213, 160},
        test_t{"F320", {70.9, -40.987, 19.73, 40}, 288, 182},
        test_t{"F640", {70.9, -40.987, 19.73, 40}, 576, 364},
    };

    for (auto& test : _test) {

        const auto& ng = key::grid::Grid::lookup(test.grid);
        const RepresentationHandle rep(ng.representation());

        log << "Test " << *rep << " with " << test.bbox << "..." << std::endl;

        // Crop to get the smallest possible bounding box
        std::vector<size_t> mapping;
        BoundingBox small(test.bbox);
        action::AreaCropper::crop(*rep, small, mapping);

        size_t n = mapping.size();
        ASSERT(0 < n);
        log << "\tnumberOfPoints =\t" << n << " (crop)" << std::endl;

        EXPECT(test.Ni * test.Nj == n);
        EXPECT(test.bbox.contains(small));

        // GRIB1/GRIB2 encoding
        EncodeRegularGaussianGrid enc(rep->croppedRepresentation(test.bbox), test.Ni, test.Nj, ng.gaussianNumber());
        EncodeRegular& encode = enc;

        for (long edition : _one_two) {

            const size_t numberOfValues = encode.numberOfValuesEncodedInGrib(edition);
            log << "\tGRIB" << edition << ": numberOfValues = " << numberOfValues << " (key)" << std::endl;
            EXPECT(numberOfValues == n);

            const size_t numberOfValuesIt = encode.numberOfValuesFromGribIterator(edition);
            log << "\tGRIB" << edition << ": numberOfValues = " << numberOfValuesIt << " (iterator)" << std::endl;
            EXPECT(numberOfValuesIt == n);

            const size_t Ni = encode.NiEncodedInGrib(edition);
            log << "\tGRIB" << edition << ": Ni = " << Ni << " (key)" << std::endl;
            EXPECT(Ni == test.Ni);

            const size_t Nj = encode.NjEncodedInGrib(edition);
            log << "\tGRIB" << edition << ": Nj = " << Nj << " (key)" << std::endl;
            EXPECT(Nj == test.Nj);

            const BoundingBox bbox = encode.boundingBoxFromGribInput(edition);
            log << "\tGRIB" << edition << ": " << bbox << std::endl;
            EXPECT(bbox.contains(small));
        }
    }
}


CASE("GRIB1/GRIB2 encoding of sub-area of regular lat/lon grids") {
    auto& log = Log::info();

    struct test_t {
        util::Increments increments;
        BoundingBox bbox;
        size_t Ni;
        size_t Nj;
    };

    // ECC-445
    std::vector<test_t> _test{
        test_t{util::Increments{0.1, 0.1}, {58.5, -6.1, 36, 20.7}, 269, 226},
    };

    for (auto& test : _test) {
        RepresentationHandle repres(new repres::latlon::RegularLL(test.increments, test.bbox));

        log << "Test " << *repres << " with " << test.bbox << "..." << std::endl;

        // Crop to get the smallest possible bounding box
        std::vector<size_t> mapping;
        BoundingBox small(test.bbox);
        action::AreaCropper::crop(*repres, small, mapping);

        size_t n = mapping.size();
        ASSERT(0 < n);
        log << "\tnumberOfPoints =\t" << n << " (crop)" << std::endl;

        EXPECT(test.Ni * test.Nj == n);
        EXPECT(test.bbox.contains(small));

        // GRIB1/GRIB2 encoding
        EncodeRegularLatLonGrid enc(repres->croppedRepresentation(test.bbox), test.Ni, test.Nj);
        EncodeRegular& encode = enc;

        for (long edition : _one_two) {

            const size_t numberOfValues = encode.numberOfValuesEncodedInGrib(edition);
            log << "\tGRIB" << edition << ": numberOfValues = " << numberOfValues << " (key)" << std::endl;
            EXPECT(numberOfValues == n);

            const size_t numberOfValuesIt = encode.numberOfValuesFromGribIterator(edition);
            log << "\tGRIB" << edition << ": numberOfValues = " << numberOfValuesIt << " (iterator)" << std::endl;
            EXPECT(numberOfValuesIt == n);

            const size_t Ni = encode.NiEncodedInGrib(edition);
            log << "\tGRIB" << edition << ": Ni = " << Ni << " (key)" << std::endl;
            EXPECT(Ni == test.Ni);

            const size_t Nj = encode.NjEncodedInGrib(edition);
            log << "\tGRIB" << edition << ": Nj = " << Nj << " (key)" << std::endl;
            EXPECT(Nj == test.Nj);

            const BoundingBox bbox = encode.boundingBoxFromGribInput(edition);
            log << "\tGRIB" << edition << ": " << bbox << std::endl;
            EXPECT(bbox.contains(small));
        }
    }
}


CASE("GRIB1/GRIB2 deleteLocalDefinition") {
    auto& log = Log::info();

    RepresentationHandle repres(new repres::latlon::RegularLL(util::Increments(1, 1)));
    log << "Test " << *(repres) << "..." << std::endl;

    // GRIB1/GRIB2 encoding
    for (bool remove : _yes_no) {
        for (long edition : _one_two) {
            util::lock_guard<util::recursive_mutex> lock(local_mutex);

            // initialise a new grib handle from samples
            grib_handle* handle(nullptr);

            grib_info info;

            // paramId "Indicates a missing value"
            auto j                                    = info.packing.extra_settings_count++;
            info.packing.extra_settings[j].name       = "paramId";
            info.packing.extra_settings[j].type       = CODES_TYPE_LONG;
            info.packing.extra_settings[j].long_value = 129255;

            info.packing.editionNumber = edition;

            info.grid.missingValue = 2.;

            // this test!
            info.packing.deleteLocalDefinition = remove ? 1 : 0;

            repres->fillGrib(info);

            size_t n = repres->numberOfPoints();
            ASSERT(n);
            std::vector<double> values(n, 0.);
            values[0] = 1.;

            // Make sure handles are deleted even in case of exception
            grib_handle* sample =
                grib_handle_new_from_samples(nullptr, ("regular_ll_pl_grib" + std::to_string(edition)).c_str());
            ASSERT(sample);
            HandleDeleter sample_detroy(sample);

            int err   = 0;
            int flags = 0;
            handle =
                codes_grib_util_set_spec(sample, &info.grid, &info.packing, flags, values.data(), values.size(), &err);
            GRIB_CALL(err);

            ASSERT(handle != nullptr);

            // initialise a new MIRInput from the grib handle

            const void* message;
            size_t length;
            GRIB_CALL(grib_get_message(handle, &message, &length));

            std::unique_ptr<MIRInput> gribInput(new input::GribMemoryInput(message, length));

            // test
            log << "\tGRIB" << edition << ": deleteLocalDefinition = " << info.packing.deleteLocalDefinition
                << std::endl;

            long remove_result = -1;
            EXPECT(codes_get_long(handle, "localUsePresent", &remove_result) == CODES_SUCCESS);

            if (remove) {
                EXPECT(remove_result == 0);
            }

            grib_handle_delete(handle);
        }
    }
}


}  // namespace mir::tests::unit

int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
