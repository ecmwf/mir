/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @file   FormulaBinop.h
/// @author Baudouin Raoult
/// @author Tiago Quintino
/// @date   April 2016

#include <iostream>
#include "mir/util/FormulaBinop.h"


namespace mir {
namespace util {

//----------------------------------------------------------------------------------------------------------------------

FormulaBinop::FormulaBinop(const std::string& name, Formula* arg1, Formula *arg2):
    FormulaFunction(name, arg1, arg2) {

}

FormulaBinop::~FormulaBinop() {
}

void FormulaBinop::print(std::ostream& out) const {
    out << '(' << *args_[0] << ") " << name_ << " (" << *args_[1] << ')' ;
}

//----------------------------------------------------------------------------------------------------------------------

} // namespace util
} // namespace mir
