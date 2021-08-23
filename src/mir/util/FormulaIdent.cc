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


namespace mir {
namespace util {


FormulaIdent::FormulaIdent(const param::MIRParametrisation& parametrisation, const std::string& name) :
    Formula(parametrisation), name_(name) {}


FormulaIdent::~FormulaIdent() = default;


void FormulaIdent::print(std::ostream& out) const {
    out << name_;
}


void FormulaIdent::execute(context::Context& ctx) const {

    // TODO: something better...

    if (name_ == "f1") {
        ctx.select(0);
        return;
    }

    if (name_ == "f2") {
        ctx.select(1);
        return;
    }

    if (name_ != "f") {
        std::ostringstream oss;
        oss << "Only variable 'f' is supported (" << name_ << ")";
        throw exception::UserError(oss.str());
    }

    // Make sure the field is loaded
    ctx.field();
}


bool FormulaIdent::sameAs(const action::Action& other) const {
    auto o = dynamic_cast<const FormulaIdent*>(&other);
    return (o != nullptr) && (name_ == o->name_);
}


const char* FormulaIdent::name() const {
    return "FormulaIdent";
}


}  // namespace util
}  // namespace mir
