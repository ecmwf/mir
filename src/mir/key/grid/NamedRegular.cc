// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/key/grid/NamedRegular.h"

#include <ostream>

#include "mir/repres/gauss/regular/RegularGG.h"
#include "mir/repres/gauss/regular/RotatedGG.h"


namespace mir::key::grid {


NamedRegular::NamedRegular(const std::string& name, size_t N) : NamedGrid(name), N_(N) {}


void NamedRegular::print(std::ostream& out) const {
    out << "NamedRegular[key=" << key_ << ",N=" << N_ << "]";
}


const repres::Representation* NamedRegular::representation() const {
    return new repres::gauss::regular::RegularGG(N_);
}


const repres::Representation* NamedRegular::representation(const util::Rotation& rotation) const {
    return new repres::gauss::regular::RotatedGG(N_, rotation);
}


size_t NamedRegular::gaussianNumber() const {
    return N_;
}


}  // namespace mir::key::grid
