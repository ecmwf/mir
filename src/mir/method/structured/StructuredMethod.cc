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


#include "mir/method/structured/StructuredMethod.h"

#include <memory>

#include "mir/repres/Iterator.h"
#include "mir/repres/Representation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"
#include "mir/util/Types.h"


namespace mir {
namespace method {
namespace structured {

StructuredMethod::StructuredMethod(const param::MIRParametrisation& param) : MethodWeighted(param) {}

StructuredMethod::~StructuredMethod() = default;

bool StructuredMethod::sameAs(const Method& other) const {
    auto o = dynamic_cast<const StructuredMethod*>(&other);
    return (o != nullptr) && MethodWeighted::sameAs(other);
}

void StructuredMethod::left_right_lon_indexes(const Longitude& in, const std::vector<PointLatLon>& coords, size_t start,
                                              size_t end, size_t& left, size_t& right) const {

    right = start;  // take the first if there's a wrap
    left  = start;

    Longitude right_lon = Longitude::GLOBE;
    //    Longitude left_lon  =   0.;
    for (size_t i = start; i < end; ++i) {

        const Longitude& lon = coords[i].lon();
        ASSERT(Longitude::GREENWICH <= lon && lon <= Longitude::GLOBE);

        if (lon <= in) {
            //            left_lon = val;
            left = i;
        }
        else if (lon < right_lon) {
            right_lon = lon;
            right     = i;
        }
    }

    ASSERT(left >= start);
    ASSERT(right >= start);
    ASSERT(right != left);
    ASSERT(coords[left].lat() == coords[right].lat());
}

void StructuredMethod::normalise(triplet_vector_t& triplets) const {
    ASSERT(triplets.size());

    // sum all calculated weights for normalisation
    double sum = 0.;
    for (const auto& t : triplets) {
        sum += t.value();
    }

    // now normalise all weights according to the total
    const double invSum = 1. / sum;
    for (auto& t : triplets) {
        t.value() *= invSum;
    }
}

void StructuredMethod::getRepresentationPoints(const repres::Representation& r, std::vector<PointLatLon>& points,
                                               Latitude& minimum, Latitude& maximum) const {
    const size_t N = r.numberOfPoints();
    points.resize(N);
    minimum = 0;
    maximum = 0;

    std::unique_ptr<repres::Iterator> it(r.iterator());
    for (size_t i = 0; it->next(); ++i) {

        ASSERT(i < N);
        const auto& p = it->pointUnrotated();

        points[i] = PointLatLon(p.lat(), p.lon());

        if (i == 0 || p.lat() < minimum) {
            minimum = p.lat();
        }

        if (i == 0 || p.lat() > maximum) {
            maximum = p.lat();
        }
    }

    ASSERT(minimum <= maximum);
}

void StructuredMethod::getRepresentationLatitudes(const repres::Representation& r,
                                                  std::vector<Latitude>& latitudes) const {
    atlas::StructuredGrid in(r.atlasGrid());
    ASSERT(in);

    const auto& pl = in.nx();
    ASSERT(pl.size() >= 2);

    latitudes.clear();
    latitudes.reserve(pl.size());

    std::unique_ptr<repres::Iterator> it(r.iterator());
    for (long Nj : pl) {
        ASSERT(Nj >= 2);
        for (long i = 0; i < Nj; ++i) {
            ASSERT(it->next());
            if (i == 0) {
                latitudes.push_back(it->pointUnrotated().lat());
            }
        }
    }

    ASSERT(!it->next());
}

void StructuredMethod::boundNorthSouth(const Latitude& lat, const std::vector<Latitude>& latitudes, size_t& jNorth,
                                       size_t& jSouth) const {
    const size_t Nj = latitudes.size();
    ASSERT(Nj > 1);

    // locate latitude indices just North and South of given latitude
    auto above = std::lower_bound(latitudes.rbegin(), latitudes.rend(), lat);

    jNorth = (Nj - static_cast<size_t>(std::distance(latitudes.rbegin(), above))) - 1;
    jSouth = jNorth + 1;

    ASSERT(/*0 <= jNorth &&*/ jNorth < Nj - 1);
    ASSERT(0 < jSouth && jSouth <= Nj - 1);
}

void StructuredMethod::boundWestEast(const Longitude& lon, const size_t& Ni, const size_t& iStart, size_t& iWest,
                                     size_t& iEast) const {
    ASSERT(Ni > 1);

    // locate longitude indices just West and East of given longitude (in-row)
    iWest = size_t(Longitude((lon * Ni) / Longitude::GLOBE.value()).value());
    iEast = (iWest + 1) % Ni;
    ASSERT(iWest < Ni);

    // adjust starting indices to value indices (absolute)
    iWest += iStart;
    iEast += iStart;
}

void StructuredMethod::assemble(util::MIRStatistics&, WeightMatrix& W, const repres::Representation& in,
                                const repres::Representation& out) const {
    Log::debug() << "StructuredMethod::assemble (input: " << in << ", output: " << out << ")..." << std::endl;

    // FIXME for the moment
    if (!in.isGlobal()) {
        throw exception::UserError("This interpolation method is only for global input grids.", Here());
    }
    if (!out.isGlobal()) {
        throw exception::UserError("This interpolation method is only for global output grids.", Here());
    }

    assembleStructuredInput(W, in, out);
    Log::debug() << "StructuredMethod::assemble." << std::endl;
}

void StructuredMethod::print(std::ostream& out) const {
    out << "StructuredMethod[";
    MethodWeighted::print(out);
    out << "]";
}

}  // namespace structured
}  // namespace method
}  // namespace mir
