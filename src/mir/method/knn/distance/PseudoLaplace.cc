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

#include "eckit/linalg/Vector.h"
#include "eckit/utils/MD5.h"

#include "mir/util/Exceptions.h"


namespace mir {
namespace method {
namespace knn {
namespace distance {


PseudoLaplace::PseudoLaplace(const param::MIRParametrisation&) {}


void PseudoLaplace::operator()(size_t ip, const Point3& point,
                               const std::vector<search::PointSearch::PointValueType>& neighbours,
                               std::vector<WeightMatrix::Triplet>& triplets) const {
    ASSERT(!neighbours.empty());
    auto npts = neighbours.size();

    triplets.clear();
    triplets.reserve(npts);

    // calculate coefficients
    eckit::linalg::Vector Dx(npts);
    eckit::linalg::Vector Dy(npts);
    eckit::linalg::Vector Dz(npts);

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
        auto& np = neighbours[j].point();

        auto dx = np[XYZCOORDS::XX] - point[XYZCOORDS::XX];
        auto dy = np[XYZCOORDS::YY] - point[XYZCOORDS::YY];
        auto dz = np[XYZCOORDS::ZZ] - point[XYZCOORDS::ZZ];

        Ixx += dx * dx;
        Ixy += dx * dy;
        Ixz += dx * dz;
        Iyy += dy * dy;
        Iyz += dy * dz;
        Izz += dz * dz;

        Rx += dx;
        Ry += dy;
        Rz += dz;

        Dx[j] = dx;
        Dy[j] = dy;
        Dz[j] = dz;
    }

    auto Lx = (-(Iyz * Iyz * Rx) + Iyy * Izz * Rx + Ixz * Iyz * Ry - Ixy * Izz * Ry - Ixz * Iyy * Rz + Ixy * Iyz * Rz) /
              (Ixz * Ixz * Iyy - 2. * Ixy * Ixz * Iyz + Ixy * Ixy * Izz + Ixx * (Iyz * Iyz - Iyy * Izz));
    auto Ly = (Ixz * Iyz * Rx - Ixy * Izz * Rx - Ixz * Ixz * Ry + Ixx * Izz * Ry + Ixy * Ixz * Rz - Ixx * Iyz * Rz) /
              (Ixz * Ixz * Iyy - 2. * Ixy * Ixz * Iyz + Ixx * Iyz * Iyz + Ixy * Ixy * Izz - Ixx * Iyy * Izz);
    auto Lz = (-(Ixz * Iyy * Rx) + Ixy * Iyz * Rx + Ixy * Ixz * Ry - Ixx * Iyz * Ry - Ixy * Ixy * Rz + Ixx * Iyy * Rz) /
              (Ixz * Ixz * Iyy - 2. * Ixy * Ixz * Iyz + Ixy * Ixy * Izz + Ixx * (Iyz * Iyz - Iyy * Izz));

    // calculate neighbour points weights, and their total (for normalisation)
    double sum = 0;
    for (size_t j = 0; j < npts; ++j) {
        auto weight = 1. + Lx * Dx[j] + Ly * Dy[j] + Lz * Dz[j];

        triplets.emplace_back(ip, neighbours[j].payload(), weight);
        sum += weight;
    }

    // normalise all weights according to the total weights sum
    ASSERT(sum > 0.);
    for (auto& t : triplets) {
        t.value() /= sum;
    }
}


bool PseudoLaplace::sameAs(const DistanceWeighting& other) const {
    return dynamic_cast<const PseudoLaplace*>(&other) != nullptr;
}


void PseudoLaplace::print(std::ostream& out) const {
    out << "PseudoLaplace[]";
}


void PseudoLaplace::hash(eckit::MD5& h) const {
    std::ostringstream s;
    s << *this;
    h.add(s.str());
}


static DistanceWeightingBuilder<PseudoLaplace> __distance("pseudo-laplace");


}  // namespace distance
}  // namespace knn
}  // namespace method
}  // namespace mir
