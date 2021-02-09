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

#include "eckit/utils/Translator.h"

#include "mir/key/grid/NamedClassic.h"


namespace mir {
namespace key {
namespace grid {


ClassicPattern::ClassicPattern(const std::string& name) : GridPattern(name) {}


ClassicPattern::~ClassicPattern() = default;


void ClassicPattern::print(std::ostream& out) const {
    out << "ClassicPattern[pattern=" << pattern_ << "]";
}


const Grid* ClassicPattern::make(const std::string& name, const param::MIRParametrisation&) const {
    return new NamedClassic(name, eckit::Translator<std::string, size_t>()(name.substr(1)));
}


static ClassicPattern __pattern("^[nN][1-9][0-9]*$");


}  // namespace grid
}  // namespace key
}  // namespace mir
