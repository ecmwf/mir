// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


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


std::string GridSpecPattern::canonical(const std::string& name) const {
    std::unique_ptr<const eckit::geo::Grid> grid(eckit::geo::GridFactory::make_from_string(name));
    return grid->spec_str();
}


static const GridSpecPattern __pattern("^[{].*[}]$");


}  // namespace mir::key::grid
