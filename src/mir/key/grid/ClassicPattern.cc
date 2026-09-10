// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/key/grid/ClassicPattern.h"

#include <ostream>

#include "mir/key/grid/NamedClassic.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Translator.h"


namespace mir::key::grid {


ClassicPattern::ClassicPattern(const std::string& pattern) : GridPattern(pattern) {}


void ClassicPattern::print(std::ostream& out) const {
    out << "ClassicPattern[pattern=" << pattern_ << "]";
}


const Grid* ClassicPattern::make(const std::string& name) const {
    return new NamedClassic(name, util::from_string<size_t>(name.substr(1)));
}


std::string ClassicPattern::canonical(const std::string& name) const {
    ASSERT(name.size() > 1);
    return "N" + name.substr(1);
}


static const ClassicPattern __pattern("^[nN][1-9][0-9]*$");


}  // namespace mir::key::grid
