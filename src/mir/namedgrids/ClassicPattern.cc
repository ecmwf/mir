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


#include "mir/namedgrids/ClassicPattern.h"

#include <iostream>

#include "eckit/utils/Translator.h"

#include "mir/namedgrids/NamedClassic.h"


namespace mir {
namespace namedgrids {


ClassicPattern::ClassicPattern(const std::string& name) : NamedGridPattern(name) {}


ClassicPattern::~ClassicPattern() = default;


void ClassicPattern::print(std::ostream& out) const {
    out << "ClassicPattern[pattern=" << pattern_ << "]";
}


const NamedGrid* ClassicPattern::make(const std::string& name) const {
    return new NamedClassic(name, eckit::Translator<std::string, size_t>()(name.substr(1)));
}


static ClassicPattern pattern("^[nN][1-9][0-9]*$");


}  // namespace namedgrids
}  // namespace mir
