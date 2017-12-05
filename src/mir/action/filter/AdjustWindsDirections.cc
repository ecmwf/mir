/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Pedro Maciel
/// @date Apr 2015


#include "mir/action/filter/AdjustWindsDirections.h"

#include <iostream>
#include <vector>

#include "eckit/exception/Exceptions.h"

#include "mir/action/context/Context.h"
#include "mir/data/MIRField.h"
#include "mir/data/CartesianVector2DField.h"
#include "mir/param/MIRParametrisation.h"


namespace mir {
namespace action {


AdjustWindsDirections::AdjustWindsDirections(const param::MIRParametrisation &parametrisation):
    Action(parametrisation) {

    std::vector<double> value;
    ASSERT(parametrisation_.userParametrisation().get("rotation", value));
    ASSERT(value.size() == 2);

    rotation_ = util::Rotation(value[0], value[1]);
}


AdjustWindsDirections::~AdjustWindsDirections() {
}


bool AdjustWindsDirections::sameAs(const Action& other) const {
    const AdjustWindsDirections* o = dynamic_cast<const AdjustWindsDirections*>(&other);
    return o && (rotation_ == o->rotation_);
}


void AdjustWindsDirections::print(std::ostream &out) const {
    out << "AdjustWindsDirections[rotation=" << rotation_ << "]";
}


void AdjustWindsDirections::execute(context::Context & ctx) const {

    data::MIRField& field = ctx.field();
    data::CartesianVector2DField cf(field.representation(), field.hasMissing(), field.missingValue());


    ASSERT((field.dimensions() % 2) == 0);
    for (size_t i = 0; i < field.dimensions(); i += 2 ) {

        // copy vector field components
        std::vector<double> valuesX = field.values(i);
        std::vector<double> valuesY = field.values(i + 1);

        cf.rotate(rotation_, valuesX, valuesY);

        field.update(valuesX, i);
        field.update(valuesY, i + 1);
    }
}

const char* AdjustWindsDirections::name() const {
    return "AdjustWindsDirections";
}

namespace {
static ActionBuilder< AdjustWindsDirections > filter("filter.adjust-winds-directions");
}


}  // namespace action
}  // namespace mir

