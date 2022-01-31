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


#include "mir/method/knn/distance/NearestLSM.h"

#include <sstream>

#include "eckit/utils/MD5.h"

#include "mir/lsm/LandSeaMasks.h"
#include "mir/method/knn/distance/DistanceWeightingWithLSM.h"
#include "mir/util/Exceptions.h"


namespace mir {
namespace method {
namespace knn {
namespace distance {


NearestLSM::NearestLSM(const param::MIRParametrisation& parametrisation, const lsm::LandSeaMasks& landSeaMasks) :
    DistanceWeightingWithLSM(parametrisation), imask_(landSeaMasks.inputMask()), omask_(landSeaMasks.outputMask()) {
    ASSERT(!imask_.empty());
    ASSERT(!omask_.empty());
}


void NearestLSM::operator()(size_t ip, const Point3&,
                            const std::vector<search::PointSearch::PointValueType>& neighbours,
                            std::vector<WeightMatrix::Triplet>& triplets) const {

    ASSERT(!neighbours.empty());
    ASSERT(ip < omask_.size());

    // choose closest neighbour point with the same output mask value
    size_t jp          = 0;
    bool foundSameType = false;

    for (const auto& p : neighbours) {
        jp = p.payload();
        ASSERT(jp < imask_.size());

        if (omask_[ip] == imask_[jp]) {
            foundSameType = true;
            break;
        }
    }

    if (!foundSameType) {
        jp = neighbours.front().payload();
    }

    triplets.assign(1, WeightMatrix::Triplet(ip, jp, 1.));
}


bool NearestLSM::sameAs(const DistanceWeighting& other) const {
    const auto* o = dynamic_cast<const NearestLSM*>(&other);
    return (o != nullptr);
}


void NearestLSM::print(std::ostream& out) const {
    out << "NearestLSM[]";
}


void NearestLSM::hash(eckit::MD5& h) const {
    std::ostringstream s;
    s << *this;
    h.add(s.str());
}


static const DistanceWeightingWithLSMBuilder<NearestLSM> __distance("nearest-lsm");


}  // namespace distance
}  // namespace knn
}  // namespace method
}  // namespace mir
