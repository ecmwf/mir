/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Tiago Quintino
/// @author Pedro Maciel
/// @date   April 2016


#include <iostream>
#include "mir/util/FormulaString.h"
#include "eckit/exception/Exceptions.h"


namespace mir {
namespace util {


FormulaString::FormulaString(const param::MIRParametrisation &parametrisation, const std::string& value):
    Formula(parametrisation),
    value_(value) {

}


FormulaString::~FormulaString() = default;


void FormulaString::print(std::ostream& out) const {
    out << "'" << value_ << "'";
}


void FormulaString::execute(mir::context::Context&) const {
    NOTIMP;
}


bool FormulaString::sameAs(const mir::action::Action& other) const {
    auto o = dynamic_cast<const FormulaString*>(&other);
    return o && (value_ == o->value_);
}


const char* FormulaString::name() const {
    return "FormulaString";
}


} // namespace util
} // namespace mir
