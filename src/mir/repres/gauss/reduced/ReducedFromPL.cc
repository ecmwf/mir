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

#include "eckit/log/JSON.h"

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

void ReducedFromPL::json(eckit::JSON& json) const {
    json.startObject();
    json << "type" << "reduced_gg";
    FromPL::json(json);
    json.endObject();
}


static const RepresentationBuilder<ReducedFromPL> reducedFromPL("reduced_gg");


}  // namespace mir::repres::gauss::reduced
