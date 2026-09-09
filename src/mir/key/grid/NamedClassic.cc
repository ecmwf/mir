// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/key/grid/NamedClassic.h"

#include <ostream>

#include "mir/repres/gauss/reduced/ReducedClassic.h"
#include "mir/repres/gauss/reduced/RotatedClassic.h"


namespace mir::key::grid {


NamedClassic::NamedClassic(const std::string& name, size_t N) : NamedGrid(name), N_(N) {}


void NamedClassic::print(std::ostream& out) const {
    out << "NamedClassic[key=" << key_ << ",N=" << N_ << "]";
}


const repres::Representation* NamedClassic::representation() const {
    return new repres::gauss::reduced::ReducedClassic(N_);
}


const repres::Representation* NamedClassic::representation(const util::Rotation& rotation) const {
    return new repres::gauss::reduced::RotatedClassic(N_, rotation);
}


size_t NamedClassic::gaussianNumber() const {
    return N_;
}


}  // namespace mir::key::grid
