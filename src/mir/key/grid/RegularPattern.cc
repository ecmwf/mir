// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/key/grid/RegularPattern.h"

#include <ostream>

#include "mir/key/grid/NamedRegular.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Translator.h"


namespace mir::key::grid {


RegularPattern::RegularPattern(const std::string& pattern) : GridPattern(pattern) {}


void RegularPattern::print(std::ostream& out) const {
    out << "RegularPattern[pattern=" << pattern_ << "]";
}


const Grid* RegularPattern::make(const std::string& name) const {
    return new NamedRegular(name, util::from_string<size_t>(name.substr(1)));
}


std::string RegularPattern::canonical(const std::string& name) const {
    ASSERT(name.size() > 1);
    return "F" + name.substr(1);
}


static const RegularPattern __pattern("^[fF][1-9][0-9]*$");


}  // namespace mir::key::grid
