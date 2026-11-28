// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <cmath>
#include <map>
#include <memory>
#include <string>

#include "eckit/geo/Grid.h"

#include "mir/util/Types.h"

#include "action.h"


namespace mir::action::tests {


/// Truncation of the input spectral fields (small, so the transforms are fast)
constexpr size_t T = 21;

/// Earth radius [m], as used by atlas/trans
constexpr double RADIUS = 6371229.;


/// Index of the real part of the spectral coefficient (m, n), in the (m-major) ordering of GRIB and atlas/trans
inline size_t index(size_t m, size_t n, size_t truncation = T) {
    ASSERT(m <= n && n <= truncation);
    size_t i = 0;

    for (size_t k = 0; k < m; ++k) {
        i += truncation + 1 - k;
    }

    return 2 * (i + n - m);
}


/// Spectral coefficients, all zero except the given ones (by index)
inline MIRValuesVector coefficients(const std::map<size_t, double>& c = {}, size_t truncation = T) {
    MIRValuesVector values((truncation + 1) * (truncation + 2), 0.);

    for (const auto& iv : c) {
        values.at(iv.first) = iv.second;
    }

    return values;
}


/// Spectral field, the same coefficients for each dimension
inline data::MIRField sh(const MIRValuesVector& values, size_t dimensions = 1, size_t truncation = T) {
    return field("{grid: T" + std::to_string(truncation) + "}", values, dimensions);
}


/// Spectral field of vorticity/divergence pairs, the same coefficients for each pair
inline data::MIRField vod(const MIRValuesVector& vo, const MIRValuesVector& d, size_t pairs = 1,
                          size_t truncation = T) {
    auto f = sh(vo, 2 * pairs, truncation);

    for (size_t p = 0; p < pairs; ++p) {
        auto copy = d;
        f.update(copy, 2 * p + 1);
    }

    return f;
}


/// Spectral coefficient (0, 0) is the global mean, (0, 1) is of the normalised harmonic sqrt(3) sin(latitude)
inline double P10(double lat) {
    return std::sqrt(3.) * std::sin(lat * M_PI / 180.);
}


/// Zonal wind of a solid body rotation, from a vorticity coefficient (0, 1): u = a vo sqrt(3)/2 cos(latitude)
inline double solid_body_u(double vo, double lat) {
    return RADIUS * vo * std::sqrt(3.) / 2. * std::cos(lat * M_PI / 180.);
}


/// Apply a function to each value of one dimension, with its geographic point (latitude, longitude)
template <typename FN>
void for_each(const data::MIRField& out, size_t which, FN fn) {
    const auto& values = out.values(which);
    size_t n           = 0;

    const auto& points = out.representation()->canonicalGrid();
    for (auto it = points.cbegin(); it; ++it, ++n) {
        const auto p = std::get<PointLonLat>(*it);
        fn(values.at(it.index()), p.lat(), p.lon());
    }

    EXPECT_EQUAL(n, values.size());
}


inline bool contains(const std::string& str, const std::string& sub) {
    return str.find(sub) != std::string::npos;
}


inline size_t numberOfPoints(const std::string& grid) {
    return representation("{grid: " + grid + "}")->numberOfPoints();
}


/// Check the output representation, against one by its gridSpec
inline void check_representation(const std::string& spec, const std::string& gridspec) {
    auto out = perform(*make(spec), sh(coefficients()));
    repres::RepresentationHandle expected(representation(gridspec));

    EXPECT_EQUAL(out.representation()->grid().spec_str(), expected->grid().spec_str());
    EXPECT_EQUAL(out.representation()->numberOfPoints(), expected->numberOfPoints());
    EXPECT_EQUAL(out.values(0).size(), expected->numberOfPoints());
}


/// Check a scalar transform: coefficient (0, 0) gives a constant field, (0, 1) a sin(latitude) field
inline void check_scalar(const action::Action& a, size_t numberOfPoints, size_t dimensions = 1) {
    auto constant = perform(a, sh(coefficients({{index(0, 0), 2.}}), dimensions));

    EXPECT_EQUAL(constant.dimensions(), dimensions);
    EXPECT_EQUAL(constant.representation()->numberOfPoints(), numberOfPoints);
    EXPECT(!constant.hasMissing());

    for (size_t which = 0; which < dimensions; ++which) {
        EXPECT_EQUAL(constant.values(which).size(), numberOfPoints);

        for_each(constant, which, [](double v, double, double) { EXPECT(same(v, 2.)); });
    }

    auto zonal = perform(a, sh(coefficients({{index(0, 1), 1.}}), dimensions));

    for (size_t which = 0; which < dimensions; ++which) {
        for_each(zonal, which, [](double v, double lat, double) { EXPECT(same(v, P10(lat))); });
    }
}


inline void check_scalar(const std::string& spec, size_t numberOfPoints, size_t dimensions = 1) {
    check_scalar(*make_checked(spec), numberOfPoints, dimensions);
}


/// Check a vorticity/divergence to u/v transform: no vorticity/divergence gives no wind, and vorticity
/// coefficient (0, 1) a solid body rotation; u/v are interleaved, with their paramId set
inline void check_vod_to_uv(const action::Action& a, size_t numberOfPoints, size_t pairs = 1) {
    auto calm = perform(a, vod(coefficients(), coefficients(), pairs));

    EXPECT_EQUAL(calm.dimensions(), 2 * pairs);
    EXPECT_EQUAL(calm.representation()->numberOfPoints(), numberOfPoints);

    for (size_t which = 0; which < 2 * pairs; ++which) {
        EXPECT_EQUAL(calm.values(which).size(), numberOfPoints);
        EXPECT_EQUAL(calm.metadata(which).at("paramId"), (which % 2 == 0 ? 131 : 132));

        for_each(calm, which, [](double v, double, double) { EXPECT(same(v, 0.)); });
    }

    constexpr double vo = 1e-5;
    auto rotation       = perform(a, vod(coefficients({{index(0, 1), vo}}), coefficients(), pairs));

    for (size_t which = 0; which < 2 * pairs; which += 2) {
        for_each(rotation, which, [](double u, double lat, double) { EXPECT(same(u, solid_body_u(vo, lat), 1e-4)); });
        for_each(rotation, which + 1, [](double v, double, double) { EXPECT(same(v, 0., 1e-4)); });
    }
}


inline void check_vod_to_uv(const std::string& spec, size_t numberOfPoints, size_t pairs = 1) {
    check_vod_to_uv(*make_checked(spec), numberOfPoints, pairs);
}


}  // namespace mir::action::tests
