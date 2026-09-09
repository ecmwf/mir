// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/key/grid/GridSpec.h"

#include <ostream>

#include "mir/util/Exceptions.h"


namespace mir::key::grid {


GridSpec::GridSpec(const std::string& key) : Grid(key, "gridspec") {}


const mir::repres::Representation* GridSpec::representation() const {
    NOTIMP;
    NOTIMP;
}


const mir::repres::Representation* GridSpec::representation(const util::Rotation&) const {
    NOTIMP;
    NOTIMP;
}


const mir::repres::Representation* GridSpec::representation(const param::MIRParametrisation&) const {
    NOTIMP;
    NOTIMP;
}


size_t GridSpec::gaussianNumber() const {
    return default_gaussian_number();
}


void GridSpec::print(std::ostream& out) const {
    out << "GridSpec[key=" << key_ << "]";
}


}  // namespace mir::key::grid
