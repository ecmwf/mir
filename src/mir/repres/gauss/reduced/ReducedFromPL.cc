// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/repres/gauss/reduced/ReducedFromPL.h"

#include <ostream>


namespace mir::repres::gauss::reduced {


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
    const auto* o = dynamic_cast<const ReducedFromPL*>(&other);
    return (o != nullptr) && FromPL::sameAs(other);
}


static const RepresentationBuilder<ReducedFromPL> reducedFromPL("reduced_gg");


}  // namespace mir::repres::gauss::reduced
