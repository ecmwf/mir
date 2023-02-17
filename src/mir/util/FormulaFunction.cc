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


#include "mir/util/FormulaFunction.h"

#include <ostream>

#include "mir/action/context/Context.h"
#include "mir/util/Function.h"


namespace mir::util {


FormulaFunction::FormulaFunction(const param::MIRParametrisation& parametrisation, const std::string& name,
                                 Formula* arg1) :
    Formula(parametrisation), function_(Function::lookup(name)) {
    args_.push_back(arg1);
}


FormulaFunction::FormulaFunction(const param::MIRParametrisation& parametrisation, const std::string& name,
                                 Formula* arg1, Formula* arg2) :
    Formula(parametrisation), function_(Function::lookup(name)) {
    args_.push_back(arg1);
    args_.push_back(arg2);
}


FormulaFunction::FormulaFunction(const param::MIRParametrisation& parametrisation, const std::string& name,
                                 std::vector<Formula*>& args) :
    Formula(parametrisation), function_(Function::lookup(name)) {
    std::swap(args_, args);
}


FormulaFunction::~FormulaFunction() {
    for (auto& j : args_) {
        delete j;
    }
}


void FormulaFunction::print(std::ostream& out) const {
    out << function_ << "(";
    const auto* sep = "";
    for (const auto& j : args_) {
        out << sep << *j;
        sep = ",";
    }
    out << ")";
}


void FormulaFunction::execute(context::Context& ctx) const {
    for (const auto& j : args_) {
        j->perform(ctx.push());
    }
    function_.execute(ctx);
}


bool FormulaFunction::sameAs(const action::Action& other) const {
    const auto* o = dynamic_cast<const FormulaFunction*>(&other);
    if ((o != nullptr) && (&function_ == &(o->function_)) && (args_.size() == o->args_.size())) {
        for (size_t i = 0; i < args_.size(); ++i) {
            if (!args_[i]->sameAs(*(o->args_[i]))) {
                return false;
            }
        }
        return true;
    }
    return false;
}


const char* FormulaFunction::name() const {
    return "FormulaFunction";
}


}  // namespace mir::util
