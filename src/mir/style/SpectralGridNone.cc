/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */

/// @date May 2017


#include "mir/style/SpectralGridNone.h"

#include <iostream>
#include "eckit/exception/Exceptions.h"


namespace mir {
namespace style {


namespace {
static SpectralGridBuilder<SpectralGridNone> __spectral_grid_none("none");
}


SpectralGridNone::SpectralGridNone(const param::MIRParametrisation& parametrisation) :
    SpectralGrid(parametrisation) {
}


void SpectralGridNone::print(std::ostream& out) const {
    out << "SpectralGridNone[]";
}


bool SpectralGridNone::active() const {
    return false;
}


std::string SpectralGridNone::getGridname() const {
    std::ostringstream os;
    os << "SpectralGridNone::getGridname() not implemented for " << *this;
    throw eckit::SeriousBug(os.str());
}


}  // namespace style
}  // namespace mir

