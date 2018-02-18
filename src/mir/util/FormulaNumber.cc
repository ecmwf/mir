/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @file   FormulaNumber.h
/// @author Baudouin Raoult
/// @author Tiago Quintino
/// @date   April 2016


#include <iostream>
#include "mir/util/FormulaNumber.h"
#include "eckit/exception/Exceptions.h"
#include "mir/action/context/Context.h"


namespace mir {
namespace util {

//----------------------------------------------------------------------------------------------------------------------
FormulaNumber::FormulaNumber(const param::MIRParametrisation &parametrisation, double value):
    Formula(parametrisation),
    value_(value) {

}

FormulaNumber::~FormulaNumber() {

}

void FormulaNumber::print(std::ostream& out) const {
    out << value_;
}


void FormulaNumber::execute(mir::context::Context& ctx) const {
    ctx.scalar(value_);
}

bool FormulaNumber::sameAs(const mir::action::Action& other) const {
    const FormulaNumber* o = dynamic_cast<const FormulaNumber*>(&other);
    return o && (value_ == o->value_);
}


const char* FormulaNumber::name() const {
    return "FormulaNumber";
}

//----------------------------------------------------------------------------------------------------------------------

} // namespace util
} // namespace mir
