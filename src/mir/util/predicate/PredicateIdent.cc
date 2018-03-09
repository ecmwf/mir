/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include <iostream>
#include "eckit/exception/Exceptions.h"
#include "mir/action/context/Context.h"
#include "mir/util/predicate/PredicateIdent.h"


namespace mir {
namespace util {
namespace predicate {


PredicateIdent::PredicateIdent(const std::string& name) :
    name_(name) {
}


PredicateIdent::~PredicateIdent() {
}


void PredicateIdent::print(std::ostream& out) const {
    out << name_;
}


void PredicateIdent::execute(mir::context::Context& ctx) const {

    // TODO: something better...

    if(name_ == "f1") {
        ctx.select(0);
        return;
    }

    if(name_ == "f2") {
        ctx.select(1);
        return;
    }

    if(name_ != "f") {
        std::ostringstream oss;
        oss << "Only variable 'f' is supported (" << name_ << ")";
        throw eckit::UserError(oss.str());
    }

    // Make sure the field is loaded
    ctx.field();
}


const char* PredicateIdent::name() const {
    return "PredicateIdent";
}


}  // namespace predicate
}  // namespace util
}  // namespace mir
