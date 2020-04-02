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


#include "mir/namedgrids/ClenshawCurtisPattern.h"

#include <ostream>

#include "mir/namedgrids/NamedClenshawCurtis.h"


namespace mir {
namespace namedgrids {


ClenshawCurtisPattern::ClenshawCurtisPattern(const std::string& name) : NamedGridPattern(name) {}


ClenshawCurtisPattern::~ClenshawCurtisPattern() = default;


void ClenshawCurtisPattern::print(std::ostream& out) const {
    out << "ClenshawCurtisPattern[pattern=" << pattern_ << "]";
}


const NamedGrid* ClenshawCurtisPattern::make(const std::string& name) const {
    return new NamedClenshawCurtis(name);
}


static ClenshawCurtisPattern pattern("^[rR][cC][cC][1-9][0-9]*$");


}  // namespace namedgrids
}  // namespace mir
