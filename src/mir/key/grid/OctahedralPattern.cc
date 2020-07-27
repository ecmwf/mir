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


#include "mir/key/grid/OctahedralPattern.h"

#include <iostream>

#include "eckit/utils/Translator.h"

#include "mir/key/grid/NamedOctahedral.h"


namespace mir {
namespace namedgrids {


OctahedralPattern::OctahedralPattern(const std::string& name) : NamedGridPattern(name) {}


OctahedralPattern::~OctahedralPattern() = default;


void OctahedralPattern::print(std::ostream& out) const {
    out << "OctahedralPattern[pattern=" << pattern_ << "]";
}


const NamedGrid* OctahedralPattern::make(const std::string& name) const {
    return new NamedOctahedral(name, eckit::Translator<std::string, size_t>()(name.substr(1)));
}


static OctahedralPattern pattern("^[oO][1-9][0-9]*$");


}  // namespace namedgrids
}  // namespace mir
