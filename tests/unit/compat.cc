// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <cstring>
#include <memory>

#include "eckit/testing/Test.h"

#include "mir/compat/GribCompatibility.h"
#include "mir/output/EmptyOutput.h"
#include "mir/param/CombinedParametrisation.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Grib.h"


namespace mir::tests::unit {


using compat::GribCompatibility;


const output::EmptyOutput OUTPUT;


long extra_setting(const grib_info& info, const char* name) {
    for (long j = 0; j < info.packing.extra_settings_count; ++j) {
        if (std::strcmp(info.packing.extra_settings[j].name, name) == 0) {
            return info.packing.extra_settings[j].long_value;
        }
    }
    return -1;
}


CASE("spectral-complex-packing") {
    const auto& compatibility = GribCompatibility::lookup("spectral-complex-packing");
    const param::SimpleParametrisation param;

    grib_info info;
    info.packing.packing      = CODES_UTIL_PACKING_USE_PROVIDED;
    info.packing.packing_type = CODES_UTIL_PACKING_TYPE_SPECTRAL_COMPLEX;

    compatibility.execute(OUTPUT, param, nullptr, info);
    EXPECT(info.packing.packing == CODES_UTIL_PACKING_USE_PROVIDED);
    EXPECT(info.packing.packing_type == CODES_UTIL_PACKING_TYPE_SPECTRAL_COMPLEX);

    info.packing.packing_type = CODES_UTIL_PACKING_TYPE_SPECTRAL_SIMPLE;

    compatibility.execute(OUTPUT, param, nullptr, info);
    EXPECT(info.packing.packing == CODES_UTIL_PACKING_USE_PROVIDED);
    EXPECT(info.packing.packing_type == CODES_UTIL_PACKING_TYPE_SPECTRAL_COMPLEX);
}


CASE("produce-large-constant-fields") {
    const auto& compatibility = GribCompatibility::lookup("produce-large-constant-fields");
    const param::SimpleParametrisation param;

    grib_info info;
    compatibility.execute(OUTPUT, param, nullptr, info);
    EXPECT_EQUAL(extra_setting(info, "produceLargeConstantFields"), 1);
}


CASE("grib1-delete-local-definition") {
    const auto& compatibility = GribCompatibility::lookup("grib1-delete-local-definition");

    for (long edition : {1, 2}) {
        param::SimpleParametrisation param;
        param.set("edition", edition);

        // the edition as requested, or as encoded
        grib_info info;
        compatibility.execute(OUTPUT, param, nullptr, info);
        EXPECT_EQUAL(info.packing.deleteLocalDefinition, (edition == 1 ? 1 : 0));

        grib_info encoded;
        encoded.packing.editionNumber = 3 - edition;
        compatibility.execute(OUTPUT, param, nullptr, encoded);
        EXPECT_EQUAL(encoded.packing.deleteLocalDefinition, (edition == 1 ? 0 : 1));
    }
}


CASE("millidegree-area") {
    const auto& compatibility = GribCompatibility::lookup("millidegree-area");

    std::unique_ptr<codes_handle, decltype(&codes_handle_delete)> h(
        codes_grib_handle_new_from_samples(nullptr, "GRIB2"), &codes_handle_delete);
    ASSERT(h);

    param::SimpleParametrisation user;
    const param::CombinedParametrisation param(user);

    SECTION("regular_ll: longitudes in [-180, 180], to the millidegree, not global nor expanded, as GRIB1") {
        grib_info info;
        info.extra_set("edition", 1L);
        info.extra_set("global", 1L);
        info.extra_set("expandBoundingBox", 1L);

        info.grid.grid_type                          = CODES_UTIL_GRID_SPEC_REGULAR_LL;
        info.grid.latitudeOfFirstGridPointInDegrees  = 60.16666666;
        info.grid.longitudeOfFirstGridPointInDegrees = -190.;
        info.grid.latitudeOfLastGridPointInDegrees   = -0.0004;
        info.grid.longitudeOfLastGridPointInDegrees  = 200.0004;

        compatibility.execute(OUTPUT, param, h.get(), info);

        EXPECT_EQUAL(extra_setting(info, "global"), 0);
        EXPECT_EQUAL(extra_setting(info, "expandBoundingBox"), 0);
        EXPECT_EQUAL(info.grid.latitudeOfFirstGridPointInDegrees, 60.167);
        EXPECT_EQUAL(info.grid.longitudeOfFirstGridPointInDegrees, 170.);
        EXPECT_EQUAL(info.grid.latitudeOfLastGridPointInDegrees, 0.);
        EXPECT_EQUAL(info.grid.longitudeOfLastGridPointInDegrees, -160.);
    }

    SECTION("reduced_ll: short of 360") {
        grib_info info;
        info.grid.grid_type                         = CODES_UTIL_GRID_SPEC_REDUCED_LL;
        info.grid.longitudeOfLastGridPointInDegrees = 359.9999;

        compatibility.execute(OUTPUT, param, h.get(), info);
        EXPECT_EQUAL(info.grid.longitudeOfLastGridPointInDegrees, 359.999);
    }

    SECTION("reduced_gg: longitudes in [0, 360), the east as the user's") {
        user.set("user-east", -10.);

        grib_info info;
        info.grid.grid_type                          = CODES_UTIL_GRID_SPEC_REDUCED_GG;
        info.grid.longitudeOfFirstGridPointInDegrees = -20.1234;
        info.grid.longitudeOfLastGridPointInDegrees  = 0.;

        compatibility.execute(OUTPUT, param, h.get(), info);

        EXPECT_EQUAL(info.grid.longitudeOfFirstGridPointInDegrees, 339.877);
        EXPECT_EQUAL(info.grid.longitudeOfLastGridPointInDegrees, 350.);

        param::SimpleParametrisation other;
        EXPECT(compatibility.sameParametrisation(param, param));
        EXPECT_NOT(compatibility.sameParametrisation(param, param::CombinedParametrisation(other)));
    }

    SECTION("unsupported") {
        grib_info info;
        info.grid.grid_type = CODES_UTIL_GRID_SPEC_HEALPIX;
        EXPECT_THROWS_AS(compatibility.execute(OUTPUT, param, h.get(), info), exception::SeriousBug);
    }
}


CASE("combined") {
    const auto& compatibility = GribCompatibility::lookup("produce-large-constant-fields/spectral-complex-packing");
    const param::SimpleParametrisation param;

    grib_info info;
    info.packing.packing      = CODES_UTIL_PACKING_USE_PROVIDED;
    info.packing.packing_type = CODES_UTIL_PACKING_TYPE_SPECTRAL_SIMPLE;

    compatibility.execute(OUTPUT, param, nullptr, info);
    EXPECT_EQUAL(extra_setting(info, "produceLargeConstantFields"), 1);
    EXPECT(info.packing.packing_type == CODES_UTIL_PACKING_TYPE_SPECTRAL_COMPLEX);
}


}  // namespace mir::tests::unit


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
