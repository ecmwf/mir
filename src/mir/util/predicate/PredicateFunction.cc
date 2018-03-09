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
#include "mir/util/predicate/Function.h"
#include "mir/util/predicate/PredicateFunction.h"

namespace mir {
namespace util {
namespace predicate {


PredicateFunction::PredicateFunction(const std::string& name, Predicate* arg1) :
    function_(Function::lookup(name)) {
    args_.push_back(arg1);
}


PredicateFunction::PredicateFunction(const std::string& name, Predicate* arg1, Predicate *arg2) :
    function_(Function::lookup(name)) {
    args_.push_back(arg1);
    args_.push_back(arg2);
}


PredicateFunction::PredicateFunction(const std::string& name, std::vector<Predicate*>& args) :
    function_(Function::lookup(name)) {
    std::swap(args_, args);
}


PredicateFunction::~PredicateFunction() {
    for (auto j = args_.begin(); j != args_.end(); ++j) {
        delete (*j);
    }
}


void PredicateFunction::print(std::ostream& out) const {
    out << function_ << "(";
    const char* sep = "";
    for (auto j = args_.begin(); j != args_.end(); ++j) {
        out << sep << *(*j);
        sep = ",";
    }

    out << ")";
}


void PredicateFunction::execute(mir::context::Context& ctx) const {

    size_t i = 0;
    for (auto j = args_.begin(); j != args_.end(); ++j, ++i) {
        (*j)->perform(ctx.push());
    }

    function_.execute(ctx);

}


const char* PredicateFunction::name() const {
    return "PredicateFunction";
}


}  // namespace predicate
}  // namespace util
}  // namespace mir
