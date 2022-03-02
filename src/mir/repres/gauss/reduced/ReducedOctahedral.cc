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


#include "mir/repres/gauss/reduced/ReducedOctahedral.h"

#include <ostream>


namespace mir {
namespace repres {
namespace gauss {
namespace reduced {


ReducedOctahedral::ReducedOctahedral(size_t N, const util::BoundingBox& bbox, double angularPrecision) :
    Octahedral(N, bbox, angularPrecision) {}


void ReducedOctahedral::print(std::ostream& out) const {
    out << "ReducedOctahedral[N=" << N_ << ",bbox=" << bbox_ << "]";
}


Iterator* ReducedOctahedral::iterator() const {
    return unrotatedIterator();
}


const Gridded* ReducedOctahedral::croppedRepresentation(const util::BoundingBox& bbox) const {
    return new ReducedOctahedral(N_, bbox, angularPrecision_);
}


void ReducedOctahedral::makeName(std::ostream& out) const {
    Octahedral::makeName(out);
}


bool ReducedOctahedral::sameAs(const Representation& other) const {
    const auto* o = dynamic_cast<const ReducedOctahedral*>(&other);
    return (o != nullptr) && Octahedral::sameAs(other);
}


}  // namespace reduced
}  // namespace gauss
}  // namespace repres
}  // namespace mir
