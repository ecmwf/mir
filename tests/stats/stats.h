// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <functional>
#include <memory>
#include <sstream>
#include <string>

#include "mir/data/MIRField.h"
#include "mir/param/GridSpecParametrisation.h"
#include "mir/repres/Representation.h"
#include "mir/util/Types.h"


namespace mir::stats::tests {


constexpr double MISSING = 9999.;


inline repres::RepresentationHandle representation(const std::string& gridspec) {
    param::GridSpecParametrisation param(gridspec);
    return repres::RepresentationFactory::build(param);
}


/// Field of a representation, with missing values where values equal MISSING
inline data::MIRField make_field(const repres::Representation* rep, const MIRValuesVector& values) {
    data::MIRField field(rep, false, MISSING);

    auto copy = values;
    field.update(copy, 0, true);

    return field;
}


/// Field of a representation by its gridSpec, values as given
inline data::MIRField make_field(const std::string& gridspec, const MIRValuesVector& values) {
    repres::RepresentationHandle rep(representation(gridspec));
    return make_field(rep, values);
}


/// Field of a gridded representation by its gridSpec, values by point (latitude, longitude)
inline data::MIRField make_field(const std::string& gridspec, const std::function<double(double, double)>& f) {
    repres::RepresentationHandle rep(representation(gridspec));

    MIRValuesVector values(rep->numberOfPoints(), 0.);
    const auto& points = rep->canonicalGridUnrotated();
    for (auto it = points.cbegin(); it; ++it) {
        const auto p          = std::get<PointLonLat>(*it);
        values.at(it.index()) = f(p.lat(), p.lon());
    }

    return make_field(rep, values);
}


template <typename T>
std::string to_string(const T& printable) {
    std::ostringstream str;
    str << printable;
    return str.str();
}


inline bool contains(const std::string& str, const std::string& sub) {
    return str.find(sub) != std::string::npos;
}


}  // namespace mir::stats::tests
