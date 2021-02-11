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

#include <ostream>

#include "eckit/utils/Translator.h"

#include "mir/key/grid/NamedOctahedral.h"
#include "mir/util/Exceptions.h"


namespace mir {
namespace key {
namespace grid {


OctahedralPattern::OctahedralPattern(const std::string& name) : GridPattern(name) {}


OctahedralPattern::~OctahedralPattern() = default;


void OctahedralPattern::print(std::ostream& out) const {
    out << "OctahedralPattern[pattern=" << pattern_ << "]";
}


const Grid* OctahedralPattern::make(const std::string& name) const {
    return new NamedOctahedral(name, eckit::Translator<std::string, size_t>()(name.substr(1)));
}

std::string OctahedralPattern::canonical(const std::string& name, const param::MIRParametrisation&) const {
    ASSERT(name.size() > 1);
    return "O" + name.substr(1);
}


static OctahedralPattern __pattern("^[oO][1-9][0-9]*$");


}  // namespace grid
}  // namespace key
}  // namespace mir
