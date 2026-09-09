// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/repres/gauss/reduced/FromPL.h"

#include "eckit/utils/MD5.h"

#include "mir/util/Domain.h"
#include "mir/util/Exceptions.h"


namespace mir::repres::gauss::reduced {


FromPL::FromPL(const param::MIRParametrisation& parametrisation) : Reduced(parametrisation) {}


FromPL::FromPL(size_t N, const std::vector<long>& pl, const util::BoundingBox& bbox, double angularPrecision) :
    Reduced(N, pl, bbox, angularPrecision) {}


void FromPL::makeName(std::ostream& out) const {
    out << "R" << N_ << "-";

    eckit::MD5 md5;
    for (const auto& j : pls()) {
        md5 << j;
    }

    out << std::string(md5);
    bbox_.makeName(out);
}


bool FromPL::sameAs(const Representation& other) const {
    const auto* o = dynamic_cast<const FromPL*>(&other);
    return (o != nullptr) && (pls() == o->pls()) && Reduced::sameAs(other);
}


atlas::Grid FromPL::atlasGrid() const {
    return atlas::ReducedGaussianGrid(pls(), domain());
}


}  // namespace mir::repres::gauss::reduced
