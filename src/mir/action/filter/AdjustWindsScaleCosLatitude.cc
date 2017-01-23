/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @date Jan 2017


#include "mir/action/filter/AdjustWindsScaleCosLatitude.h"

#include <algorithm>
#include <cmath>
#include <functional>
#include <iostream>
#include "eckit/exception/Exceptions.h"
#include "eckit/types/FloatCompare.h"
#include "mir/action/context/Context.h"
#include "mir/data/MIRField.h"
#include "mir/repres/Iterator.h"
#include "mir/repres/Representation.h"
#include "mir/util/Angles.h"


namespace mir {
namespace action {


AdjustWindsScaleCosLatitude::AdjustWindsScaleCosLatitude(const param::MIRParametrisation &parametrisation):
    Action(parametrisation) {
}


bool AdjustWindsScaleCosLatitude::sameAs(const Action& other) const {
    const AdjustWindsScaleCosLatitude* o = dynamic_cast<const AdjustWindsScaleCosLatitude*>(&other);
    return o;
}


void AdjustWindsScaleCosLatitude::print(std::ostream &out) const {
    out << "AdjustWindsScaleCosLatitude[]";
}


void AdjustWindsScaleCosLatitude::execute(context::Context& ctx) const {
    using eckit::types::is_approximately_equal;

    data::MIRField& field = ctx.field();
    ASSERT(field.dimensions() > 0);


    // determine scaling vector (all fields share the same representation)
    const size_t N = field.values(0).size();
    ASSERT(N > 0);

    const repres::Representation* representation(field.representation());
    ASSERT(representation);

    eckit::ScopedPtr<repres::Iterator> iter(representation->unrotatedIterator());
    std::vector<double> scale(N);
    double lat = 0;
    double lon = 0;
    for (std::vector<double>::iterator s = scale.begin(); s != scale.end() && iter->next(lat, lon); ++s) {
        *s = is_approximately_equal(lat, -90.)? 0.
           : is_approximately_equal(lat,  90.)? 0.
           : 1./std::cos( util::angles::degree_to_radian(lat) );
    }


    // apply scaling to each field values vector
    for (size_t i = 0; i < field.dimensions(); ++i ) {
        std::vector<double> values = field.values(i);
        ASSERT(values.size() == N);

        std::transform(values.begin(), values.end(), scale.begin(), values.begin(), std::multiplies<double>());

        field.update(values, i);
    }
}


namespace {
static ActionBuilder< AdjustWindsScaleCosLatitude > filter("filter.adjust-winds-scale-cos-latitude");
}


}  // namespace action
}  // namespace mir

