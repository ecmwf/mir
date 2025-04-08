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


#include "mir/key/grid/GridSpecPattern.h"

#include <memory>
#include <ostream>

#include "eckit/geo/Grid.h"

#include "mir/key/grid/GridSpec.h"


namespace mir::key::grid {


GridSpecPattern::GridSpecPattern(const std::string& pattern) : GridPattern(pattern) {}


void GridSpecPattern::print(std::ostream& out) const {
    out << "GridSpecPattern[pattern=" << pattern_ << "]";
}


const Grid* GridSpecPattern::make(const std::string& name) const {
    return new GridSpec(name);
}


std::string GridSpecPattern::canonical(const std::string& name, const param::MIRParametrisation&) const {
    std::unique_ptr<const eckit::geo::Grid> grid(eckit::geo::GridFactory::make_from_string(name));
    return grid->spec_str();
}


static const GridSpecPattern __pattern("^[{].*[}]$");


}  // namespace mir::key::grid
