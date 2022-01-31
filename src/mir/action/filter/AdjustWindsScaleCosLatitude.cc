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


#include "mir/action/filter/AdjustWindsScaleCosLatitude.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <memory>
#include <ostream>

#include "mir/action/context/Context.h"
#include "mir/data/MIRField.h"
#include "mir/repres/Iterator.h"
#include "mir/repres/Representation.h"
#include "mir/util/Angles.h"
#include "mir/util/Exceptions.h"


namespace mir {
namespace action {


AdjustWindsScaleCosLatitude::AdjustWindsScaleCosLatitude(const param::MIRParametrisation& parametrisation) :
    Action(parametrisation) {}


bool AdjustWindsScaleCosLatitude::sameAs(const Action& other) const {
    const auto* o = dynamic_cast<const AdjustWindsScaleCosLatitude*>(&other);
    return (o != nullptr);
}


void AdjustWindsScaleCosLatitude::print(std::ostream& out) const {
    out << "AdjustWindsScaleCosLatitude[]";
}


void AdjustWindsScaleCosLatitude::execute(context::Context& ctx) const {
    data::MIRField& field = ctx.field();
    ASSERT(field.dimensions() > 0);


    // determine scaling vector (all fields share the same representation)
    const size_t N = field.values(0).size();
    ASSERT(N > 0);

    const repres::Representation* representation(field.representation());
    ASSERT(representation);

    std::vector<double> scale(N, std::numeric_limits<double>::quiet_NaN());
    for (const std::unique_ptr<repres::Iterator> it(representation->iterator()); it->next();) {
        const auto& p      = it->pointUnrotated();
        scale[it->index()] = (p.lat() == Latitude::SOUTH_POLE) ? 0.
                             : (p.lat() == Latitude::NORTH_POLE)
                                 ? 0.
                                 : 1. / std::cos(util::degree_to_radian(p.lat().value()));
    }

    // apply scaling to each field component directly
    for (size_t i = 0; i < field.dimensions(); ++i) {
        auto& values = field.direct(i);
        ASSERT(values.size() == N);

        std::transform(
            values.begin(), values.end(), scale.begin(), values.begin(), [](const double& a, const double& b) {
                return std::isfinite(a) && std::isfinite(b) ? a * b : std::numeric_limits<double>::quiet_NaN();
            });
    }
}

const char* AdjustWindsScaleCosLatitude::name() const {
    return "AdjustWindsScaleCosLatitude";
}


static const ActionBuilder<AdjustWindsScaleCosLatitude> __action("filter.adjust-winds-scale-cos-latitude");


}  // namespace action
}  // namespace mir
