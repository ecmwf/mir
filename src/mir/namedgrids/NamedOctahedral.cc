/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Pedro Maciel
/// @date Apr 2015



#include "mir/namedgrids/NamedOctahedral.h"
#include "mir/repres/gauss/reduced/ReducedOctahedral.h"
#include "mir/repres/gauss/reduced/RotatedOctahedral.h"
#include "eckit/exception/Exceptions.h"
#include <iostream>

namespace mir {
namespace namedgrids {


NamedOctahedral::NamedOctahedral(const std::string &name, size_t N):
    NamedGrid(name),
    N_(N) {
}


NamedOctahedral::~NamedOctahedral() {
}

void NamedOctahedral::print(std::ostream &out) const {
    out << "NamedOctahedral[name=" << name_ << ",N=" << N_ << "]";
}

const repres::Representation *NamedOctahedral::representation() const {
    return new repres::gauss::reduced::ReducedOctahedral(N_);

}

const repres::Representation *NamedOctahedral::representation(const util::Rotation &rotation) const {
    return new repres::gauss::reduced::RotatedOctahedral(N_, util::BoundingBox(), rotation);
}

size_t NamedOctahedral::gaussianNumber() const {
    return N_;
}

}  // namespace namedgrids
}  // namespace mir

