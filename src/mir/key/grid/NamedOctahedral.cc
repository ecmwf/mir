// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/key/grid/NamedOctahedral.h"

#include <ostream>

#include "mir/repres/gauss/reduced/ReducedOctahedral.h"
#include "mir/repres/gauss/reduced/RotatedOctahedral.h"


namespace mir::key::grid {


NamedOctahedral::NamedOctahedral(const std::string& name, size_t N) : NamedGrid(name), N_(N) {}


void NamedOctahedral::print(std::ostream& out) const {
    out << "NamedOctahedral[key=" << key_ << ",N=" << N_ << "]";
}


const repres::Representation* NamedOctahedral::representation() const {
    return new repres::gauss::reduced::ReducedOctahedral(N_);
}


const repres::Representation* NamedOctahedral::representation(const util::Rotation& rotation) const {
    return new repres::gauss::reduced::RotatedOctahedral(N_, rotation);
}


size_t NamedOctahedral::gaussianNumber() const {
    return N_;
}


}  // namespace mir::key::grid
