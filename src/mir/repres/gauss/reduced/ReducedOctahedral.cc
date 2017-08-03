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


#include "mir/repres/gauss/reduced/ReducedOctahedral.h"

#include <iostream>
#include "mir/repres/gauss/reduced/ReducedFromPL.h"


namespace mir {
namespace repres {
namespace gauss {
namespace reduced {


ReducedOctahedral::ReducedOctahedral(size_t N):
    Octahedral(N) {
}


ReducedOctahedral::~ReducedOctahedral() {
}


void ReducedOctahedral::print(std::ostream &out) const {
    out << "ReducedOctahedral[N" << N_ << ",bbox=" << bbox_ << "]";
}


Iterator *ReducedOctahedral::iterator() const {
    return unrotatedIterator();
}


const Reduced *ReducedOctahedral::cropped(const util::BoundingBox &bbox, const std::vector<long> &pl) const {
    // We lose the ReducedOctahedral nature of the grid
    return new ReducedFromPL(N_, pl, bbox);
}


void ReducedOctahedral::makeName(std::ostream& out) const {
    Octahedral::makeName(out);
}


bool ReducedOctahedral::sameAs(const Representation& other) const {
    const ReducedOctahedral* o = dynamic_cast<const ReducedOctahedral*>(&other);
    return o && Octahedral::sameAs(other);
}


}  // namespace reduced
}  // namespace gauss
}  // namespace repres
}  // namespace mir

