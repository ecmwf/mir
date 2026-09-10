// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/repres/gauss/reduced/ReducedOctahedral.h"

#include <ostream>


namespace mir::repres::gauss::reduced {


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


}  // namespace mir::repres::gauss::reduced
