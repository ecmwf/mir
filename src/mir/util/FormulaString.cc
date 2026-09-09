// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/util/FormulaString.h"

#include <ostream>

#include "mir/util/Exceptions.h"


namespace mir::util {


FormulaString::FormulaString(const param::MIRParametrisation& parametrisation, const std::string& value) :
    Formula(parametrisation), value_(value) {}


FormulaString::~FormulaString() = default;


void FormulaString::print(std::ostream& out) const {
    out << "'" << value_ << "'";
}


void FormulaString::execute(context::Context& /*unused*/) const {
    NOTIMP;
}


bool FormulaString::sameAs(const action::Action& other) const {
    const auto* o = dynamic_cast<const FormulaString*>(&other);
    return (o != nullptr) && (value_ == o->value_);
}


const char* FormulaString::name() const {
    return "FormulaString";
}


}  // namespace mir::util
