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


#include "mir/key/grid/ORCAPattern.h"

#include <ostream>

#include "mir/key/grid/NamedORCA.h"


namespace mir {
namespace key {
namespace grid {


ORCAPattern::ORCAPattern(const std::string& name) : GridPattern(name) {}


static const std::string __orca_pattern = "^(e?[oO][rR][cC][aA][0-9]+)_([TUVWF])$";


const std::string& ORCAPattern::pattern() {
    return __orca_pattern;
}


ORCAPattern::~ORCAPattern() = default;


void ORCAPattern::print(std::ostream& out) const {
    out << "ORCAPattern[pattern=" << pattern_ << "]";
}


const Grid* ORCAPattern::make(const std::string& name) const {
    return new NamedORCA(name);
}


static ORCAPattern __pattern(__orca_pattern);


}  // namespace grid
}  // namespace key
}  // namespace mir
