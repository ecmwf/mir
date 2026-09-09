// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/key/grid/OctahedralPattern.h"

#include <ostream>

#include "mir/key/grid/NamedOctahedral.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Translator.h"


namespace mir::key::grid {


OctahedralPattern::OctahedralPattern(const std::string& pattern) : GridPattern(pattern) {}


void OctahedralPattern::print(std::ostream& out) const {
    out << "OctahedralPattern[pattern=" << pattern_ << "]";
}


const Grid* OctahedralPattern::make(const std::string& name) const {
    return new NamedOctahedral(name, util::from_string<size_t>(name.substr(1)));
}

std::string OctahedralPattern::canonical(const std::string& name) const {
    ASSERT(name.size() > 1);
    return "O" + name.substr(1);
}


static const OctahedralPattern __pattern("^[oO][1-9][0-9]*$");


}  // namespace mir::key::grid
