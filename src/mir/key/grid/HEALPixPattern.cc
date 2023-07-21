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


#include "mir/key/grid/HEALPixPattern.h"

#include <ostream>

#include "eckit/utils/Translator.h"

#include "mir/key/grid/NamedHEALPix.h"
#include "mir/util/Exceptions.h"


namespace mir::key::grid {


HEALPixPattern::HEALPixPattern(const std::string& name) : GridPattern(name) {}


HEALPixPattern::~HEALPixPattern() = default;


void HEALPixPattern::print(std::ostream& out) const {
    out << "HEALPixPattern[pattern=" << pattern_ << "]";
}


const Grid* HEALPixPattern::make(const std::string& name) const {
    return new NamedHEALPix(name, eckit::Translator<std::string, size_t>()(name.substr(1)));
}


std::string HEALPixPattern::canonical(const std::string& name, const param::MIRParametrisation& param) const {
    ASSERT(name.size() > 1);
    return "H" + name.substr(1);
}


static const HEALPixPattern __pattern1("^[hH][1-9][0-9]*$");

}  // namespace mir::key::grid
