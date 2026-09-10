// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/repres/gauss/reduced/ReducedClassic.h"

#include <ostream>


namespace mir::repres::gauss::reduced {


ReducedClassic::ReducedClassic(size_t N, const util::BoundingBox& bbox, double angularPrecision) :
    Classic(N, bbox, angularPrecision) {}


void ReducedClassic::print(std::ostream& out) const {
    out << "ReducedClassic[N=" << N_ << ",bbox=" << bbox_ << "]";
}


Iterator* ReducedClassic::iterator() const {
    return unrotatedIterator();
}


const Gridded* ReducedClassic::croppedRepresentation(const util::BoundingBox& bbox) const {
    return new ReducedClassic(N_, bbox, angularPrecision_);
}


void ReducedClassic::makeName(std::ostream& out) const {
    Classic::makeName(out);
}


bool ReducedClassic::sameAs(const Representation& other) const {
    const auto* o = dynamic_cast<const ReducedClassic*>(&other);
    return (o != nullptr) && Classic::sameAs(other);
}


}  // namespace mir::repres::gauss::reduced
