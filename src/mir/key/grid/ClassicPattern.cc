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


#include "mir/key/grid/ClassicPattern.h"

#include <ostream>

#include "mir/key/grid/NamedClassic.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Translator.h"


namespace mir::key::grid {


ClassicPattern::ClassicPattern(const std::string& pattern) : GridPattern(pattern) {}


void ClassicPattern::print(std::ostream& out) const {
    out << "ClassicPattern[pattern=" << pattern_ << "]";
}


const Grid* ClassicPattern::make(const std::string& name) const {
    return new NamedClassic(name, util::from_string<size_t>(name.substr(1)));
}


std::string ClassicPattern::canonical(const std::string& name, const param::MIRParametrisation& /*unused*/) const {
    ASSERT(name.size() > 1);
    return "N" + name.substr(1);
}


static const ClassicPattern __pattern("^[nN][1-9][0-9]*$");


}  // namespace mir::key::grid
