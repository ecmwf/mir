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
#include <functional>
#include <iostream>
#include <memory>

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
    auto o = dynamic_cast<const AdjustWindsScaleCosLatitude*>(&other);
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

    std::unique_ptr<repres::Iterator> iter(representation->iterator());

    std::vector<double> scale(N);
    for (auto& s : scale) {
        ASSERT(iter->next());
        const auto& p = iter->pointUnrotated();
        s             = (p.lat() == Latitude::SOUTH_POLE)   ? 0.
                        : (p.lat() == Latitude::NORTH_POLE) ? 0.
                                                            : 1. / std::cos(util::degree_to_radian(p.lat().value()));
    }
    ASSERT(!(iter->next()));


    // apply scaling to each field component directly
    for (size_t i = 0; i < field.dimensions(); ++i) {
        MIRValuesVector& values = field.direct(i);
        ASSERT(values.size() == N);

        std::transform(values.begin(), values.end(), scale.begin(), values.begin(), std::multiplies<double>());
    }
}

const char* AdjustWindsScaleCosLatitude::name() const {
    return "AdjustWindsScaleCosLatitude";
}


static ActionBuilder<AdjustWindsScaleCosLatitude> __action("filter.adjust-winds-scale-cos-latitude");


}  // namespace action
}  // namespace mir
