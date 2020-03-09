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


#include "mir/namedgrids/ORCAPattern.h"

#include <iostream>

#include "mir/namedgrids/NamedORCA.h"


namespace mir {
namespace namedgrids {


ORCAPattern::ORCAPattern(const std::string& name) : NamedGridPattern(name) {}


ORCAPattern::~ORCAPattern() = default;


void ORCAPattern::print(std::ostream& out) const {
    out << "ORCAPattern[pattern=" << pattern_ << "]";
}


const NamedGrid* ORCAPattern::make(const std::string& name) const {
    return new NamedORCA(name);
}


static ORCAPattern pattern("^e?orca[0-9]+_[FTUV]$");


}  // namespace namedgrids
}  // namespace mir
