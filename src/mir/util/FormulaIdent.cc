/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @file   FormulaIdent.h
/// @author Baudouin Raoult
/// @author Tiago Quintino
/// @date   April 2016

#include <iostream>
#include "mir/util/FormulaIdent.h"
#include "eckit/exception/Exceptions.h"


namespace mir {
namespace util {

//----------------------------------------------------------------------------------------------------------------------
FormulaIdent::FormulaIdent(const param::MIRParametrisation &parametrisation, const std::string& name):
    Formula(parametrisation),
    name_(name) {

}

FormulaIdent::~FormulaIdent() {

}

void FormulaIdent::print(std::ostream& out) const {
    out << name_;
}


void FormulaIdent::execute(mir::context::Context&) const {
    NOTIMP;
}

bool FormulaIdent::sameAs(const mir::action::Action& other) const {
    const FormulaIdent* o = dynamic_cast<const FormulaIdent*>(&other);
    return o && (name_ == o->name_);
}

//----------------------------------------------------------------------------------------------------------------------

} // namespace util
} // namespace mir
