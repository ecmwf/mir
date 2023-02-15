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
