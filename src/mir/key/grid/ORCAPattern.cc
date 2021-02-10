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


std::string ORCAPattern::sane_name(const std::string& insane) {
    auto s = insane;
    std::transform(s.begin(), s.end(), s.begin(), ::toupper);
    if (s.front() == 'E') {
        s.front() = 'e';
    }
    return s;
}


void ORCAPattern::print(std::ostream& out) const {
    out << "ORCAPattern[pattern=" << pattern_ << "]";
}


const Grid* ORCAPattern::make(const std::string& name, const param::MIRParametrisation& param) const {
    if (name.find('_') == std::string::npos) {
        std::string arrangement;
        param.get("orca-arrangement", arrangement = "T");  // arbitrary choice (to review)

        return new NamedORCA(sane_name(name + "_" + arrangement));
    }

    return new NamedORCA(sane_name(name));
}


static ORCAPattern __pattern("^[eE]?[oO][rR][cC][aA][0-9]+(|_[tTuUvVwWfF])$");


}  // namespace grid
}  // namespace key
}  // namespace mir
