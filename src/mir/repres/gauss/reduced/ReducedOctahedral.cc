/*
 * (C) Copyright 1996- ECMWF.
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


namespace mir {
namespace repres {
namespace gauss {
namespace reduced {


ReducedOctahedral::ReducedOctahedral(size_t N, const util::BoundingBox& bbox):
    Octahedral(N, bbox) {
}


ReducedOctahedral::~ReducedOctahedral() = default;


void ReducedOctahedral::print(std::ostream &out) const {
    out << "ReducedOctahedral[N=" << N_ << ",bbox=" << bbox_ << "]";
}


Iterator *ReducedOctahedral::iterator() const {
    return unrotatedIterator();
}


const Gridded* ReducedOctahedral::croppedRepresentation(const util::BoundingBox& bbox) const {
    return new ReducedOctahedral(N_, bbox);
}


void ReducedOctahedral::makeName(std::ostream& out) const {
    Octahedral::makeName(out);
}


bool ReducedOctahedral::sameAs(const Representation& other) const {
    auto o = dynamic_cast<const ReducedOctahedral*>(&other);
    return o && Octahedral::sameAs(other);
}


}  // namespace reduced
}  // namespace gauss
}  // namespace repres
}  // namespace mir

