// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/repres/gauss/reduced/Classic.h"

#include "mir/util/Domain.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"
#include "mir/util/Types.h"


namespace mir::repres::gauss::reduced {


Classic::Classic(size_t N, const util::BoundingBox& bbox, double angularPrecision) :
    Reduced(N, bbox, angularPrecision) {

    // adjust latitudes, longitudes and re-set bounding box
    auto n = bbox.north();
    auto s = bbox.south();
    correctSouthNorth(s, n);

    setNj(pls("N" + std::to_string(N_)), s, n);

    auto w = bbox.west();
    auto e = bbox.east();
    correctWestEast(w, e);

    auto old(bbox_);
    bbox_ = util::BoundingBox(n, w, s, e);
    Log::debug() << "Classic BoundingBox:"
                 << "\n\t   " << old << "\n\t > " << bbox_ << std::endl;
}


void Classic::makeName(std::ostream& out) const {
    out << "N" << N_;
    bbox_.makeName(out);
}


bool Classic::sameAs(const Representation& other) const {
    const auto* o = dynamic_cast<const Classic*>(&other);
    return (o != nullptr) && Reduced::sameAs(other);
}


atlas::Grid Classic::atlasGrid() const {
    return atlas::ReducedGaussianGrid("N" + std::to_string(N_), domain());
}


}  // namespace mir::repres::gauss::reduced
