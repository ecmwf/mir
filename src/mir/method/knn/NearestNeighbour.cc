/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Tiago Quintino
/// @author Pedro Maciel
/// @date May 2015


#include "mir/method/knn/NearestNeighbour.h"

#include "eckit/utils/MD5.h"
#include "mir/method/knn/distance/DistanceWeighting.h"
#include "mir/param/MIRParametrisation.h"


namespace mir {
namespace method {
namespace knn {


NearestNeighbour::NearestNeighbour(const param::MIRParametrisation& param) : KNearestNeighbours(param), pick_(param) {}


void NearestNeighbour::assemble(util::MIRStatistics& stats, WeightMatrix& W, const repres::Representation& in,
                                const repres::Representation& out) const {
    assembleCustomised(stats, W, in, out, pick(), distanceWeighting());
}


void NearestNeighbour::print(std::ostream& out) const {
    out << "NearestNeighbour[";
    MethodWeighted::print(out);
    out << ",nearestMethod=" << pick() << ",distanceWeighting=" << distanceWeighting() << "]";
}


NearestNeighbour::~NearestNeighbour() = default;


bool NearestNeighbour::sameAs(const Method& other) const {
    auto o = dynamic_cast<const NearestNeighbour*>(&other);
    return o && pick().sameAs(o->pick()) && KNearestNeighbours::sameAs(other);
}


const pick::Pick& NearestNeighbour::pick() const {
    return pick_;
}


const char* NearestNeighbour::name() const {
    return "nearest-neighbour";
}


namespace {
static MethodBuilder<NearestNeighbour> __method1("nearest-neighbour");
static MethodBuilder<NearestNeighbour> __method2("nearest-neighbor");  // For the americans
static MethodBuilder<NearestNeighbour> __method3("nn");                // For the lazy
}  // namespace


}  // namespace knn
}  // namespace method
}  // namespace mir
