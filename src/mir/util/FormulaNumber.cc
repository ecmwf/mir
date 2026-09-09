// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/util/FormulaNumber.h"

#include <ostream>

#include "mir/action/context/Context.h"


namespace mir::util {


FormulaNumber::FormulaNumber(const param::MIRParametrisation& parametrisation, double value) :
    Formula(parametrisation), value_(value) {}

FormulaNumber::~FormulaNumber() = default;

void FormulaNumber::print(std::ostream& out) const {
    out << value_;
}


void FormulaNumber::execute(context::Context& ctx) const {
    ctx.scalar(value_);
}

bool FormulaNumber::sameAs(const action::Action& other) const {
    const auto* o = dynamic_cast<const FormulaNumber*>(&other);
    return (o != nullptr) && (value_ == o->value_);
}


const char* FormulaNumber::name() const {
    return "FormulaNumber";
}


}  // namespace mir::util
