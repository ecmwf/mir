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


#include "mir/key/grid/RegularLLPattern.h"

#include <ostream>

#include "mir/key/grid/RegularLL.h"


namespace mir {
namespace key {
namespace grid {


RegularLLPattern::RegularLLPattern(const std::string& name) : GridPattern(name) {}


RegularLLPattern::~RegularLLPattern() = default;


void RegularLLPattern::print(std::ostream& out) const {
    out << "RegularLLPattern[pattern=" << pattern_ << "]";
}


const Grid* RegularLLPattern::make(const std::string& name, const param::MIRParametrisation&) const {
    return new grid::RegularLL(name);
}


#define fp "[+]?([0-9]*[.])?[0-9]+([eE][-+][0-9]+)?"
static RegularLLPattern __pattern("^" fp "/" fp "$");
#undef fp


}  // namespace grid
}  // namespace key
}  // namespace mir
