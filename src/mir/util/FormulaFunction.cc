/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @file   FormulaFunction.h
/// @author Baudouin Raoult
/// @author Tiago Quintino
/// @date   April 2016

#include <iostream>
#include "mir/util/FormulaFunction.h"


namespace mir {
namespace util {

//----------------------------------------------------------------------------------------------------------------------

FormulaFunction::FormulaFunction(const std::string& name, Formula* arg1): name_(name) {
    args_.push_back(arg1);
}

FormulaFunction::FormulaFunction(const std::string& name, Formula* arg1, Formula *arg2): name_(name) {
    args_.push_back(arg1);
    args_.push_back(arg2);
}

FormulaFunction::FormulaFunction(const std::string& name, std::vector<Formula*>& args): name_(name) {
    std::swap(args_, args);
}

FormulaFunction::~FormulaFunction() {
}

void FormulaFunction::print(std::ostream& out) const {
    out << name_ << "(";
    const char* sep = "";
    for (auto j = args_.begin(); j != args_.end(); ++j) {
        out << sep << *(*j);
        sep = ",";
    }

    out << ")";
}

//----------------------------------------------------------------------------------------------------------------------

} // namespace util
} // namespace mir
