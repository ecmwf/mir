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



#include "mir/namedgrids/FullPattern.h"
#include "mir/namedgrids/NamedFull.h"
#include "eckit/utils/Translator.h"

#include <iostream>

namespace mir {
namespace namedgrids {


FullPattern::FullPattern(const std::string &name):
    NamedGridPattern(name) {
}


FullPattern::~FullPattern() {
}

void FullPattern::print(std::ostream &out) const {
    out << "FullPattern[pattern=" <<  pattern_ << "]";
}

const NamedGrid *FullPattern::make(const std::string& name) const {
    return new NamedFull(name, eckit::Translator<std::string, size_t>()(name.substr(1)));
}

namespace {
static FullPattern pattern("^[fF][1-9][0-9]*$");

}


}  // namespace logic
}  // namespace mir

