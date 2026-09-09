// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/repres/gauss/reduced/Octahedral.h"

#include "mir/util/Domain.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"
#include "mir/util/MeshGeneratorParameters.h"
#include "mir/util/Types.h"


namespace mir::repres::gauss::reduced {


Octahedral::Octahedral(size_t N, const util::BoundingBox& bbox, double angularPrecision) :
    Reduced(N, bbox, angularPrecision) {

    // adjust latitudes, longitudes and re-set bounding box
    Latitude n = bbox.north();
    Latitude s = bbox.south();
    correctSouthNorth(s, n);

    setNj(pls("O" + std::to_string(N_)), s, n);

    Longitude w = bbox.west();
    Longitude e = bbox.east();
    correctWestEast(w, e);

    auto old(bbox_);
    bbox_ = util::BoundingBox(n, w, s, e);
    Log::debug() << "Octahedral BoundingBox:"
                 << "\n\t   " << old << "\n\t > " << bbox_ << std::endl;
}


void Octahedral::makeName(std::ostream& out) const {
    out << "O" << N_;
    bbox_.makeName(out);
}


bool Octahedral::sameAs(const Representation& other) const {
    const auto* o = dynamic_cast<const Octahedral*>(&other);
    return (o != nullptr) && Reduced::sameAs(other);
}


atlas::Grid Octahedral::atlasGrid() const {
    return atlas::ReducedGaussianGrid("O" + std::to_string(N_), domain());
}


void Octahedral::fillMeshGen(util::MeshGeneratorParameters& params) const {
    Gaussian::fillMeshGen(params);
    params.set("triangulate", true);
}


}  // namespace mir::repres::gauss::reduced
