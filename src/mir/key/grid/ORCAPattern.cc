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

#include <algorithm>
#include <cctype>
#include <ostream>

#include "mir/key/grid/NamedORCA.h"


namespace mir {
namespace key {
namespace grid {


ORCAPattern::ORCAPattern(const std::string& name) : GridPattern(name) {}


ORCAPattern::~ORCAPattern() = default;


void ORCAPattern::print(std::ostream& out) const {
    out << "ORCAPattern[pattern=" << pattern_ << "]";
}


const Grid* ORCAPattern::make(const std::string& name) const {
    return new NamedORCA(name);
}


std::string ORCAPattern::canonical(const std::string& name, const param::MIRParametrisation& param) const {
    auto n = name;
    if (n.find('_') == std::string::npos) {
        std::string arrangement = "T";  // arbitrary choice (to review)
        param.get("orca-arrangement", arrangement);
        n += "_" + arrangement;
    }

    std::transform(n.begin(), n.end(), n.begin(), ::toupper);
    if (n.front() == 'E') {
        n.front() = 'e';
    }

    return n;
}


static const ORCAPattern __pattern1("^[eE]?[oO][rR][cC][aA][0-9]+$");
static const ORCAPattern __pattern2("^[eE]?[oO][rR][cC][aA][0-9]+_[tTuUvVwWfF]$");


}  // namespace grid
}  // namespace key
}  // namespace mir
