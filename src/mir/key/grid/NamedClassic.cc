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


#include "mir/key/grid/NamedClassic.h"

#include <iostream>

#include "mir/repres/gauss/reduced/ReducedClassic.h"
#include "mir/repres/gauss/reduced/RotatedClassic.h"


namespace mir {
namespace key {
namespace grid {


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


}  // namespace grid
}  // namespace key
}  // namespace mir
