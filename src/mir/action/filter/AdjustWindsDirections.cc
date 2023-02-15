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


#include "mir/action/filter/AdjustWindsDirections.h"

#include <ostream>
#include <vector>

#include "mir/action/context/Context.h"
#include "mir/data/CartesianVector2DField.h"
#include "mir/data/MIRField.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/util/Exceptions.h"


namespace mir::action {


AdjustWindsDirections::AdjustWindsDirections(const param::MIRParametrisation& parametrisation) :
    Action(parametrisation) {
    std::vector<double> rotation;
    ASSERT(parametrisation_.userParametrisation().get("rotation", rotation));
    ASSERT_KEYWORD_ROTATION_SIZE(rotation.size());

    rotation_ = util::Rotation(rotation[0], rotation[1]);
}


AdjustWindsDirections::~AdjustWindsDirections() = default;


bool AdjustWindsDirections::sameAs(const Action& other) const {
    const auto* o = dynamic_cast<const AdjustWindsDirections*>(&other);
    return (o != nullptr) && (rotation_ == o->rotation_);
}


void AdjustWindsDirections::print(std::ostream& out) const {
    out << "AdjustWindsDirections[rotation=" << rotation_ << "]";
}


void AdjustWindsDirections::execute(context::Context& ctx) const {
    data::MIRField& field = ctx.field();
    data::CartesianVector2DField cf(field.representation(), field.hasMissing(), field.missingValue());

    ASSERT((field.dimensions() % 2) == 0);
    for (size_t i = 0; i < field.dimensions(); i += 2) {

        // set field components directly
        MIRValuesVector& valuesX = field.direct(i);
        MIRValuesVector& valuesY = field.direct(i + 1);

        cf.rotate(rotation_, valuesX, valuesY);

        field.metadata(i, "uvRelativeToGrid", 1);
        field.metadata(i + 1, "uvRelativeToGrid", 1);
    }
}


const char* AdjustWindsDirections::name() const {
    return "AdjustWindsDirections";
}


static const ActionBuilder<AdjustWindsDirections> __action("filter.adjust-winds-directions");


}  // namespace mir::action
