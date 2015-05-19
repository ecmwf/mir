/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Pedro Maciel
/// @date Apr 2015


#include "mir/param/option/Separator.h"
#include "eckit/exception/Exceptions.h"
#include "eckit/utils/Translator.h"

#include <iostream>

namespace mir {
namespace param {
namespace option {


Separator::Separator(const std::string &description):
    Option("", description) {
}


Separator::~Separator() {
}


void Separator::set(const std::string &value, SimpleParametrisation &parametrisation) const {
    NOTIMP;
}

bool Separator::active() const {
    return false;
}

void Separator::print(std::ostream &out) const {
    out << std::endl << description_ << ":" << std::endl;
}

} // namespace option
}  // namespace param
}  // namespace mir

