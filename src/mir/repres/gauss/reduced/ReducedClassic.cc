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
