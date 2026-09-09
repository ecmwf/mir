// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/key/grid/RegularLLPattern.h"

#include <ostream>
#include <sstream>

#include "eckit/utils/StringTools.h"

#include "mir/key/grid/RegularLL.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Translator.h"


namespace mir::key::grid {


RegularLLPattern::RegularLLPattern(const std::string& pattern) : GridPattern(pattern) {}


void RegularLLPattern::print(std::ostream& out) const {
    out << "RegularLLPattern[pattern=" << pattern_ << "]";
}


const Grid* RegularLLPattern::make(const std::string& name) const {
    return new grid::RegularLL(name);
}


std::string RegularLLPattern::canonical(const std::string& name) const {
    auto split = eckit::StringTools::split("/", name);
    ASSERT(split.size() == 2);

    std::ostringstream str;
    str << util::from_string<double>(split[0]) << '/'
        << util::from_string<double>(split[1]);  // better than using std::to_string
    return str.str();
}


#define fp "[+]?([0-9]*[.])?[0-9]+([eE][-+][0-9]+)?"
static const RegularLLPattern __pattern("^" fp "/" fp "$");
#undef fp


}  // namespace mir::key::grid
