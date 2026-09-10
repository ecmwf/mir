// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/action/misc/SelectField.h"

#include <ostream>

#include "mir/action/context/Context.h"
#include "mir/data/MIRField.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/util/Exceptions.h"


namespace mir::action {


SelectField::SelectField(const param::MIRParametrisation& param) : Action(param) {
    ASSERT(parametrisation().get("which", which_));
}


bool SelectField::sameAs(const Action& other) const {
    const auto* o = dynamic_cast<const SelectField*>(&other);
    return (o != nullptr) && (which_ == o->which_);
}


void SelectField::print(std::ostream& out) const {
    out << "SelectField[" << which_ << "]";
}


void SelectField::execute(context::Context& ctx) const {
    data::MIRField& field = ctx.field();
    field.select(which_);
}


const char* SelectField::name() const {
    return "SelectField";
}


static const ActionBuilder<SelectField> __action("select.field");


}  // namespace mir::action
