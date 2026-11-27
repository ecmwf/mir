// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <cmath>
#include <memory>
#include <string>
#include <vector>

#include "eckit/geo/Point.h"
#include "eckit/testing/Test.h"
#include "eckit/utils/StringTools.h"

#include "mir/data/MIRField.h"
#include "mir/input/GribMemoryInput.h"
#include "mir/param/SimpleParametrisation.h"

#include "repres.h"


namespace mir::repres::tests {


// a key and its value (long if integral)
struct kv_t {
    std::string key;
    double value;
};


// a point (latitude, longitude)
struct point_t {
    double lat;
    double lon;
};


using keys_t   = std::vector<kv_t>;
using points_t = std::vector<point_t>;


inline keys_t operator+(keys_t a, const keys_t& b) {
    a.insert(a.end(), b.begin(), b.end());
    return a;
}


/// GRIB2 message of a gridType (by sample) and its keys, of n values (the point index), or (n = 0) of a single value
/// for Nx * Ny points
inline std::string message(const std::string& gridType, const keys_t& keys, long n = 0) {
    std::unique_ptr<codes_handle, decltype(&codes_handle_delete)> h(
        codes_grib_handle_new_from_samples(nullptr, "GRIB2"), &codes_handle_delete);
    ASSERT(h);

    GRIB_CALL(codes_set_string(h.get(), "gridType", gridType.c_str(), nullptr));

    for (const auto& kv : keys) {
        if (kv.value == std::round(kv.value)) {
            GRIB_CALL(codes_set_long(h.get(), kv.key.c_str(), static_cast<long>(kv.value)));
        }
        else {
            GRIB_CALL(codes_set_double(h.get(), kv.key.c_str(), kv.value));
        }
    }

    std::vector<double> v(n > 0 ? static_cast<size_t>(n) : 1, 1.);
    for (size_t i = 0; n > 0 && i < v.size(); ++i) {
        v[i] = static_cast<double>(i);
    }

    if (n == 0) {
        long nx = 0;
        long ny = 0;
        GRIB_CALL(codes_get_long(h.get(), "Nx", &nx));
        GRIB_CALL(codes_get_long(h.get(), "Ny", &ny));
        n = nx * ny;
    }

    // as grib_filter: values before the number of points

    GRIB_CALL(codes_set_double_array(h.get(), "values", v.data(), v.size()));
    GRIB_CALL(codes_set_long(h.get(), "numberOfDataPoints", n));
    GRIB_CALL(codes_set_long(h.get(), "numberOfValues", n));

    const void* msg = nullptr;
    size_t length   = 0;
    GRIB_CALL(codes_get_message(h.get(), &msg, &length));

    return {static_cast<const char*>(msg), length};
}


inline RepresentationHandle representation(const std::string& msg) {
    input::GribMemoryInput input(msg.data(), msg.size());
    const input::MIRInput& in = input;
    return in.field().representation();
}


inline points_t points(const Representation& repres) {
    points_t visited;
    for (const auto& point : repres.canonicalGridUnrotated()) {
        const PointLatLon p{std::get<PointLonLat>(point)};
        visited.push_back({p.lat().value(), p.lon().value()});
    }
    return visited;
}


/// Representation by the keys of a GRIB grid type (gridType=...;key=value;...), as GRIB without a gridSpec
inline RepresentationHandle by_keys(const std::string& keys) {
    param::SimpleParametrisation param;
    for (const auto& kv : eckit::StringTools::split(";", keys)) {
        const auto i = kv.find('=');
        ASSERT(i != std::string::npos);
        param.set(kv.substr(0, i), kv.substr(i + 1));
    }
    return RepresentationFactory::build(param);
}


const std::string LAEA =
    "gridType=lambert_azimuthal_equal_area;grid=100000/100000;standardParallelInDegrees=52;"
    "centralLongitudeInDegrees=10;latitudeOfFirstGridPointInDegrees=66.982143;"
    "longitudeOfFirstGridPointInDegrees=-35.034024;Ni=40;Nj=38";

const std::string POLAR_STEREOGRAPHIC =
    "gridType=polar_stereographic;LaDInDegrees=60;orientationOfTheGridInDegrees=0;southPoleOnProjectionPlane=0;"
    "grid=100000/100000;latitudeOfFirstGridPointInDegrees=60;longitudeOfFirstGridPointInDegrees=-45;Ni=20;Nj=20";

const std::string LAMBERT =
    "gridType=lambert;LaDInDegrees=50;LoVInDegrees=10;Latin1InDegrees=50;Latin2InDegrees=50;grid=100000/100000;"
    "latitudeOfFirstGridPointInDegrees=40;longitudeOfFirstGridPointInDegrees=-10;Ni=20;Nj=15";


}  // namespace mir::repres::tests
