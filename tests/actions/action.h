// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <cmath>
#include <functional>
#include <memory>
#include <string>

#include "eckit/testing/Test.h"

#include "mir/action/context/Context.h"
#include "mir/action/plan/Action.h"
#include "mir/data/MIRField.h"
#include "mir/param/GridSpecParametrisation.h"
#include "mir/repres/Iterator.h"
#include "mir/repres/Representation.h"
#include "mir/util/MIRStatistics.h"
#include "mir/util/Types.h"


namespace mir::tests::actions {


using A = std::unique_ptr<const action::Action>;
using F = std::function<double(double lat, double lon)>;


/// Action by its spec (see action::ActionFactory::make_from_string)
inline A make(const std::string& spec) {
    return A(action::ActionFactory::make_from_string(spec));
}


/// Action by its spec, checking its complete spec builds the same action
inline A make_checked(const std::string& spec) {
    auto a = make(spec);
    auto b = make(a->json_str(false));

    EXPECT(*a == *b);
    EXPECT_EQUAL(b->json_str(false), a->json_str(false));

    return a;
}


/// Representation by its gridSpec
inline repres::RepresentationHandle representation(const std::string& gridspec) {
    param::GridSpecParametrisation param(gridspec);
    return repres::RepresentationFactory::build(param);
}


/// Field of a representation, values (the same for each dimension) as given, with missing values if any equal
/// missingValue (default 9999, a value test functions avoid)
inline data::MIRField field(const repres::Representation* repres, const MIRValuesVector& values, size_t dimensions = 1,
                            double missingValue = 9999.) {
    data::MIRField field(repres, false, missingValue);

    for (size_t which = 0; which < dimensions; ++which) {
        auto copy = values;
        field.update(copy, which, true);
    }

    return field;
}


/// Field of a gridded representation (by its gridSpec), values by point (latitude, longitude), or zero
inline data::MIRField field(
    const std::string& gridspec, const F& f = [](double, double) { return 0.; }, size_t dimensions = 1,
    double missingValue = 9999.) {
    repres::RepresentationHandle repres(representation(gridspec));

    // by point index, as iterators might not visit points in storage order (eg. HEALPix)
    MIRValuesVector values(repres->numberOfPoints(), 0.);

    for (std::unique_ptr<repres::Iterator> it(repres->iterator()); it->next();) {
        const auto& p          = it->pointUnrotated();
        values.at(it->index()) = f ? f(p.lat().value(), p.lon().value()) : 0.;
    }

    return field(repres, values, dimensions, missingValue);
}


/// Field of a representation (by its gridSpec), values as given (eg. spectral coefficients)
inline data::MIRField field(const std::string& gridspec, const MIRValuesVector& values, size_t dimensions = 1,
                            double missingValue = 9999.) {
    repres::RepresentationHandle repres(representation(gridspec));
    return field(repres, values, dimensions, missingValue);
}


/// Action performed on (a copy of) the field
inline data::MIRField perform(const action::Action& action, const data::MIRField& input) {
    data::MIRField copy(input);
    util::MIRStatistics statistics;
    context::Context ctx(copy, statistics);
    action.perform(ctx);
    return ctx.field();
}


/// Action (by its spec) performed on (a copy of) the field
inline data::MIRField perform(const std::string& spec, const data::MIRField& input) {
    return perform(*make_checked(spec), input);
}


inline size_t count_missing(const data::MIRField& field, size_t which = 0) {
    size_t n = 0;

    for (const auto& v : field.values(which)) {
        n += v == field.missingValue() ? 1 : 0;
    }

    return n;
}


inline bool same(double a, double b, double eps = 1e-9) {
    return std::abs(a - b) <= eps;
}


}  // namespace mir::tests::actions
