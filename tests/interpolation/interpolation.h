// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <cmath>
#include <functional>
#include <memory>
#include <string>

#include "eckit/testing/Test.h"

#include "mir/action/context/Context.h"
#include "mir/data/MIRField.h"
#include "mir/method/Method.h"
#include "mir/method/MethodWeighted.h"
#include "mir/param/GridSpecParametrisation.h"
#include "mir/repres/Iterator.h"
#include "mir/repres/Representation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/MIRStatistics.h"
#include "mir/util/Types.h"


namespace mir::interpolation::tests {


using M = std::unique_ptr<method::Method>;
using F = std::function<double(double lat, double lon)>;


/// Method by its spec (see method::MethodFactory::make_from_string), without (disk) caching unless specified, so
/// matrices are computed
inline M make(const std::string& spec) {
    if (spec.find("caching") != std::string::npos) {
        return M(method::MethodFactory::make_from_string(spec));
    }

    const auto with = spec.empty() || spec.front() != '{' ? "{type: " + spec + ", caching: false}"
                                                          : R"({"caching": false, )" + spec.substr(1);
    return M(method::MethodFactory::make_from_string(with));
}


/// Representation by its gridSpec
inline repres::RepresentationHandle representation(const std::string& gridspec) {
    param::GridSpecParametrisation param(gridspec);
    return repres::RepresentationFactory::build(param);
}


/// Field of a representation, values as given, with missing values if any equal missingValue
inline data::MIRField field(const repres::Representation* repres, const MIRValuesVector& values,
                            double missingValue = 9999.) {
    data::MIRField field(repres, false, missingValue);

    auto copy = values;
    field.update(copy, 0, true);

    return field;
}


/// Field of a representation, values by point (latitude, longitude)
inline data::MIRField field(const repres::Representation* repres, const F& f) {
    MIRValuesVector values(repres->numberOfPoints(), 0.);

    // by point index, as iterators might not visit points in storage order (eg. HEALPix)
    for (std::unique_ptr<repres::Iterator> it(repres->iterator()); it->next();) {
        const auto& p          = it->pointUnrotated();
        values.at(it->index()) = f(p.lat().value(), p.lon().value());
    }

    return field(repres, values);
}


/// Field interpolated by the method, from the field representation to the given representation
inline data::MIRField interpolate(const method::Method& method, const data::MIRField& input,
                                  const repres::Representation& out) {
    data::MIRField copy(input);
    util::MIRStatistics statistics;
    context::Context ctx(copy, statistics);

    repres::RepresentationHandle in(input.representation());
    method.execute(ctx, *in, out);

    ctx.field().representation(&out);
    return ctx.field();
}


/// Weight matrix of a matrix-based method, from one representation to another
inline method::WeightMatrix matrix(const method::Method& method, const repres::Representation& in,
                                   const repres::Representation& out) {
    const auto* weighted = dynamic_cast<const method::MethodWeighted*>(&method);
    ASSERT(weighted != nullptr);

    data::MIRField dummy(&in);
    util::MIRStatistics statistics;
    context::Context ctx(dummy, statistics);

    return weighted->getMatrix(ctx, in, out);
}


/// Matrix entry (zero if not stored)
inline double weight(const method::WeightMatrix& W, size_t row, size_t col) {
    for (auto it = W.begin(row); it != W.end(row); ++it) {
        if (it.col() == col) {
            return *it;
        }
    }
    return 0.;
}


inline size_t count_missing(const data::MIRField& field) {
    size_t n = 0;
    for (const auto& v : field.values(0)) {
        n += v == field.missingValue() ? 1 : 0;
    }
    return n;
}


inline bool same(double a, double b, double eps = 1e-9) {
    return std::abs(a - b) <= eps;
}


}  // namespace mir::interpolation::tests
