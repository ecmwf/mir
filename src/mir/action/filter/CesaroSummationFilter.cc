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


#include "mir/action/filter/CesaroSummationFilter.h"

#include <algorithm>
#include <ostream>
#include <vector>

#include "mir/action/context/Context.h"
#include "mir/api/MIREstimation.h"
#include "mir/data/MIRField.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/sh/SphericalHarmonics.h"
#include "mir/util/Exceptions.h"


namespace mir {
namespace action {
namespace filter {


CesaroSummationFilter::CesaroSummationFilter(const param::MIRParametrisation& parametrisation) :
    Action(parametrisation), k_(2.), Tmin_(1) {
    parametrisation.get("cesaro-k", k_);
    ASSERT(0. <= k_);

    parametrisation.get("cesaro-truncation", Tmin_);
    ASSERT(1 <= Tmin_);
}


CesaroSummationFilter::~CesaroSummationFilter() = default;


bool CesaroSummationFilter::sameAs(const Action& other) const {
    auto o = dynamic_cast<const CesaroSummationFilter*>(&other);
    return (o != nullptr) && (k_ == o->k_) && (Tmin_ == o->Tmin_);
}


void CesaroSummationFilter::print(std::ostream& out) const {
    out << "CesaroSummationFilter[k=" << k_ << ",truncation=" << Tmin_ << "]";
}


void CesaroSummationFilter::execute(context::Context& ctx) const {
    data::MIRField& field = ctx.field();

    size_t T = field.representation()->truncation();
    ASSERT(Tmin_ <= T);

    size_t N = repres::sh::SphericalHarmonics::number_of_complex_coefficients(T);
    ASSERT(N > 0);

    std::vector<double> filter(T + 1);

    std::fill_n(filter.begin(), Tmin_, 1.);
    for (size_t n = Tmin_; n <= T; ++n) {
        auto a    = double(T - n + 1);
        auto f    = filter[n - 1];
        filter[n] = f * a / (a + k_);
    }

    for (size_t d = 0; d < field.dimensions(); ++d) {
        auto& values = field.direct(d);
        ASSERT(values.size() == N * 2);

        for (size_t m = 0, k = 0; m <= T; ++m) {
            for (size_t n = m; n <= T; ++n) {
                ASSERT(k + 1 < N * 2);
                values[k++] *= filter[n];
                values[k++] *= filter[n];
            }
        }
    }
}


const char* CesaroSummationFilter::name() const {
    return "CesaroSummationFilter";
}


void CesaroSummationFilter::estimate(context::Context&, api::MIREstimation& estimation) const {
    estimation.sameAsInput();
}


static ActionBuilder<CesaroSummationFilter> __action("filter.sh-cesaro-summation-filter");


}  // namespace filter
}  // namespace action
}  // namespace mir
