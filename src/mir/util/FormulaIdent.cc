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


#include "mir/util/FormulaIdent.h"

#include <ostream>
#include <sstream>

#include "mir/action/context/Context.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Regex.h"


namespace mir::util {


FormulaIdent::FormulaIdent(const param::MIRParametrisation& parametrisation, const std::string& name) :
    Formula(parametrisation), name_(name) {}


FormulaIdent::~FormulaIdent() = default;


void FormulaIdent::print(std::ostream& out) const {
    out << name_;
}


void FormulaIdent::execute(context::Context& ctx) const {
    // TODO: something better...

    const auto match = Regex::match("f([0-9]+)", name_);
    if (match) {
        ASSERT(match.size() == 2);

        size_t which = 0;
        std::istringstream iss(match[1]);
        iss >> which;
        ASSERT(which > 0);

        ctx.select(which - 1);
        return;
    }

    if (name_ != "f") {
        std::ostringstream oss;
        oss << "Only variables 'f', 'f1' (same as 'f'), 'f2', 'f3'... are supported (" << name_ << ")";
        throw exception::UserError(oss.str());
    }

    // Make sure the field is loaded
    ctx.field();
}


bool FormulaIdent::sameAs(const action::Action& other) const {
    const auto* o = dynamic_cast<const FormulaIdent*>(&other);
    return (o != nullptr) && (name_ == o->name_);
}


const char* FormulaIdent::name() const {
    return "FormulaIdent";
}


}  // namespace mir::util
