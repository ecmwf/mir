/*
 * (C) Copyright 1996- ECMWF.
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



#include "mir/namedgrids/RegularPattern.h"
#include "mir/namedgrids/NamedRegular.h"
#include "eckit/utils/Translator.h"

#include <iostream>

namespace mir {
namespace namedgrids {


RegularPattern::RegularPattern(const std::string &name):
    NamedGridPattern(name) {
}


RegularPattern::~RegularPattern() {
}

void RegularPattern::print(std::ostream &out) const {
    out << "RegularPattern[pattern=" <<  pattern_ << "]";
}

const NamedGrid *RegularPattern::make(const std::string& name) const {
    return new NamedRegular(name, eckit::Translator<std::string, size_t>()(name.substr(1)));
}

namespace {
static RegularPattern pattern("^[fF][1-9][0-9]*$");

}


}  // namespace namedgrids
}  // namespace mir

