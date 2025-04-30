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


#include "mir/method/knn/distance/PseudoLaplace.h"

#include <sstream>

#include "eckit/log/JSON.h"

#include "eckit/types/FloatCompare.h"
#include "eckit/utils/MD5.h"

#include "mir/util/Exceptions.h"


namespace mir::method::knn::distance {


PseudoLaplace::PseudoLaplace(const param::MIRParametrisation& /*unused*/) {}


void PseudoLaplace::operator()(size_t ip, const PointXYZ& point,
                               const std::vector<search::PointSearch::PointValueType>& neighbours,
                               std::vector<WeightMatrix::Triplet>& triplets) const {
    ASSERT(!neighbours.empty());
    auto npts = neighbours.size();

    triplets.clear();
    triplets.reserve(npts);

    // calculate coefficients
    std::vector<PointXYZ> D(npts);

    double Ixx = 0;
    double Ixy = 0;
    double Ixz = 0;
    double Iyy = 0;
    double Iyz = 0;
    double Izz = 0;
    double Rx  = 0;
    double Ry  = 0;
    double Rz  = 0;

    for (size_t j = 0; j < npts; ++j) {
        D[j] = neighbours[j].point() - point;

        auto dx = D[j].X;
        auto dy = D[j].Y;
        auto dz = D[j].Z;

        Ixx += dx * dx;
        Ixy += dx * dy;
        Ixz += dx * dz;
        Iyy += dy * dy;
        Iyz += dy * dz;
        Izz += dz * dz;

        Rx += dx;
        Ry += dy;
        Rz += dz;
    }

    // calculate neighbour points weights, and their total (for normalisation)
    // on moments singularities, degenerate into nearest neighbour
    auto a = Ixz * Ixz * Iyy - 2. * Ixy * Ixz * Iyz + Ixy * Ixy * Izz + Ixx * (Iyz * Iyz - Iyy * Izz);
    auto b = Ixz * Ixz * Iyy - 2. * Ixy * Ixz * Iyz + Ixx * Iyz * Iyz + Ixy * Ixy * Izz - Ixx * Iyy * Izz;

    if (eckit::types::is_approximately_equal(a, 0.) || eckit::types::is_approximately_equal(b, 0.)) {
        triplets.emplace_back(ip, neighbours[0].payload(), 1.);
        return;
    }

    PointXYZ L{
        (-(Iyz * Iyz * Rx) + Iyy * Izz * Rx + Ixz * Iyz * Ry - Ixy * Izz * Ry - Ixz * Iyy * Rz + Ixy * Iyz * Rz) / a,
        (Ixz * Iyz * Rx - Ixy * Izz * Rx - Ixz * Ixz * Ry + Ixx * Izz * Ry + Ixy * Ixz * Rz - Ixx * Iyz * Rz) / b,
        (-(Ixz * Iyy * Rx) + Ixy * Iyz * Rx + Ixy * Ixz * Ry - Ixx * Iyz * Ry - Ixy * Ixy * Rz + Ixx * Iyy * Rz) / a};

    double sum = 0;
    for (size_t j = 0; j < npts; ++j) {
        auto weight = 1. + dot(L, D[j]);
        triplets.emplace_back(ip, neighbours[j].payload(), weight);
        sum += weight;
    }

    if (eckit::types::is_approximately_equal(sum, 0.)) {
        triplets = {{ip, neighbours[0].payload(), 1.}};
        return;
    }

    for (auto& t : triplets) {
        t.value() /= sum;
    }
}


bool PseudoLaplace::sameAs(const DistanceWeighting& other) const {
    return dynamic_cast<const PseudoLaplace*>(&other) != nullptr;
}


void PseudoLaplace::json(eckit::JSON& j) const {
    j.startObject();
    j << "type"
      << "pseudo-laplace";
    j.endObject();
}


void PseudoLaplace::print(std::ostream& out) const {
    out << "PseudoLaplace[]";
}


void PseudoLaplace::hash(eckit::MD5& h) const {
    std::ostringstream s;
    s << *this;
    h.add(s.str());
}


WeightMatrix::Check PseudoLaplace::validateMatrixWeights() const {
    // this method does not produce bounded interpolation weights
    return {true, false, false};
}


static const DistanceWeightingBuilder<PseudoLaplace> __distance("pseudo-laplace");


}  // namespace mir::method::knn::distance
