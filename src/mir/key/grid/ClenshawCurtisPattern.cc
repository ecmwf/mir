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


#include "mir/key/grid/ClenshawCurtisPattern.h"

#include <ostream>

#include "mir/key/grid/NamedClenshawCurtis.h"
#include "mir/util/Exceptions.h"


namespace mir {
namespace key {
namespace grid {


ClenshawCurtisPattern::ClenshawCurtisPattern(const std::string& name) : GridPattern(name) {}


ClenshawCurtisPattern::~ClenshawCurtisPattern() = default;


void ClenshawCurtisPattern::print(std::ostream& out) const {
    out << "ClenshawCurtisPattern[pattern=" << pattern_ << "]";
}


const Grid* ClenshawCurtisPattern::make(const std::string& name) const {
    return new NamedClenshawCurtis(name);
}


std::string ClenshawCurtisPattern::canonical(const std::string& name, const param::MIRParametrisation&) const {
    ASSERT(name.size() > 3);
    return "RCC" + name.substr(3);
}


static ClenshawCurtisPattern __pattern("^[rR][cC][cC][1-9][0-9]*$");


}  // namespace grid
}  // namespace key
}  // namespace mir
