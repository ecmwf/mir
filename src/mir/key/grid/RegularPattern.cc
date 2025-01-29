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


#include "mir/key/grid/RegularPattern.h"

#include <ostream>

#include "eckit/utils/Translator.h"

#include "mir/key/grid/NamedRegular.h"
#include "mir/util/Exceptions.h"


namespace mir::key::grid {


RegularPattern::RegularPattern(const std::string& pattern) : GridPattern(pattern) {}


void RegularPattern::print(std::ostream& out) const {
    out << "RegularPattern[pattern=" << pattern_ << "]";
}


const Grid* RegularPattern::make(const std::string& name) const {
    return new NamedRegular(name, eckit::Translator<std::string, size_t>()(name.substr(1)));
}


std::string RegularPattern::canonical(const std::string& name, const param::MIRParametrisation& /*unused*/) const {
    ASSERT(name.size() > 1);
    return "F" + name.substr(1);
}


static const RegularPattern __pattern("^[fF][1-9][0-9]*$");


}  // namespace mir::key::grid
