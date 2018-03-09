/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/util/predicate/PredicateNumber.h"

#include <iostream>
#include "eckit/exception/Exceptions.h"
#include "mir/action/context/Context.h"


namespace mir {
namespace util {
namespace predicate {


PredicateNumber::PredicateNumber(double value):
    Predicate(),
    value_(value) {
}


PredicateNumber::~PredicateNumber() {
}


void PredicateNumber::print(std::ostream& out) const {
    out << value_;
}


void PredicateNumber::execute(mir::context::Context& ctx) const {
    ctx.scalar(value_);
}


const char* PredicateNumber::name() const {
    return "PredicateNumber";
}


}  // namespace predicate
}  // namespace util
}  // namespace mir
