/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/util/predicate/PredicateString.h"

#include <iostream>
#include "eckit/exception/Exceptions.h"


namespace mir {
namespace util {
namespace predicate {


PredicateString::PredicateString(const std::string& value):
    Predicate(),
    value_(value) {
}


PredicateString::~PredicateString() {
}


void PredicateString::print(std::ostream& out) const {
    out << "'" << value_ << "'";
}


void PredicateString::execute(mir::context::Context&) const {
    NOTIMP;
}


const char* PredicateString::name() const {
    return "PredicateString";
}


}  // namespace predicate
}  // namespace util
}  // namespace mir
