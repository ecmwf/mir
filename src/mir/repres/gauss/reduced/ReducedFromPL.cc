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


#include "mir/repres/gauss/reduced/ReducedFromPL.h"

#include <ostream>


namespace mir {
namespace repres {
namespace gauss {
namespace reduced {


ReducedFromPL::ReducedFromPL(const param::MIRParametrisation& parametrisation) : FromPL(parametrisation) {}


ReducedFromPL::ReducedFromPL(size_t N, const std::vector<long>& pl, const util::BoundingBox& bbox,
                             double angularPrecision) :
    FromPL(N, pl, bbox, angularPrecision) {}


void ReducedFromPL::print(std::ostream& out) const {
    out << "ReducedFromPL[N=" << N_ << ",bbox=" << bbox_ << "]";
}


Iterator* ReducedFromPL::iterator() const {
    return unrotatedIterator();
}


const Gridded* ReducedFromPL::croppedRepresentation(const util::BoundingBox& bbox) const {
    return new ReducedFromPL(N_, pls(), bbox, angularPrecision_);
}


void ReducedFromPL::makeName(std::ostream& out) const {
    FromPL::makeName(out);
}


bool ReducedFromPL::sameAs(const Representation& other) const {
    auto o = dynamic_cast<const ReducedFromPL*>(&other);
    return (o != nullptr) && FromPL::sameAs(other);
}


static RepresentationBuilder<ReducedFromPL> reducedFromPL("reduced_gg");


}  // namespace reduced
}  // namespace gauss
}  // namespace repres
}  // namespace mir
