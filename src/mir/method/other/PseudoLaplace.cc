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


#include "mir/method/other/PseudoLaplace.h"

#include <string>

#include "eckit/linalg/Vector.h"
#include "eckit/utils/MD5.h"

#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Iterator.h"
#include "mir/repres/Representation.h"
#include "mir/search/PointSearch.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"
#include "mir/util/Trace.h"


namespace mir {
namespace method {
namespace other {


PseudoLaplace::PseudoLaplace(const param::MIRParametrisation& param) : MethodWeighted(param), nclosest_(4) {

    param.get("nclosest", nclosest_);
}


PseudoLaplace::~PseudoLaplace() = default;


const char* PseudoLaplace::name() const {
    return "pseudo-laplace";
}


void PseudoLaplace::hash(eckit::MD5& md5) const {
    MethodWeighted::hash(md5);
    md5 << nclosest_;
}

bool PseudoLaplace::sameAs(const Method& other) const {
    auto o = dynamic_cast<const PseudoLaplace*>(&other);
    return (o != nullptr) && (nclosest_ == o->nclosest_) && MethodWeighted::sameAs(other);
}

void PseudoLaplace::assemble(util::MIRStatistics&, WeightMatrix& W, const repres::Representation& in,
                             const repres::Representation& out) const {
    Log::debug() << "PseudoLaplace::assemble (input: " << in << ", output: " << out << ")" << std::endl;
    trace::Timer timer("PseudoLaplace::assemble");


    search::PointSearch sptree(parametrisation_, in);

    const size_t out_npts = out.numberOfPoints();

    // init structure used to fill in sparse matrix
    std::vector<WeightMatrix::Triplet> weights_triplets;
    weights_triplets.reserve(out_npts * nclosest_);

    std::vector<search::PointSearch::PointValueType> closest;

    eckit::linalg::Vector Dx(nclosest_);
    eckit::linalg::Vector Dy(nclosest_);
    eckit::linalg::Vector Dz(nclosest_);

    std::vector<double> weights;
    weights.reserve(nclosest_);

    for (const std::unique_ptr<repres::Iterator> it(out.iterator()); it->next();) {
        auto ip = it->index();
        ASSERT(ip < out_npts);

        // get the reference output point
        Point3 p(it->point3D());

        // find the closest input points to this output
        sptree.closestNPoints(p, nclosest_, closest);

        const size_t npts = closest.size();

        // then calculate the nearest neighbour weights
        weights.resize(npts, 0.0);

        double Ixx(0);
        double Ixy(0);
        double Ixz(0);
        double Iyy(0);
        double Iyz(0);
        double Izz(0);
        double Rx(0);
        double Ry(0);
        double Rz(0);
        double Lx;
        double Ly;
        double Lz;

        for (size_t j = 0; j < npts; ++j) {
            Point3 np = closest[j].point();

            const double dx = np[XYZCOORDS::XX] - p[XYZCOORDS::XX];
            const double dy = np[XYZCOORDS::YY] - p[XYZCOORDS::YY];
            const double dz = np[XYZCOORDS::ZZ] - p[XYZCOORDS::ZZ];

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

        Lx = (-(Iyz * Iyz * Rx) + Iyy * Izz * Rx + Ixz * Iyz * Ry - Ixy * Izz * Ry - Ixz * Iyy * Rz + Ixy * Iyz * Rz) /
             (Ixz * Ixz * Iyy - 2. * Ixy * Ixz * Iyz + Ixy * Ixy * Izz + Ixx * (Iyz * Iyz - Iyy * Izz));
        Ly = (Ixz * Iyz * Rx - Ixy * Izz * Rx - Ixz * Ixz * Ry + Ixx * Izz * Ry + Ixy * Ixz * Rz - Ixx * Iyz * Rz) /
             (Ixz * Ixz * Iyy - 2. * Ixy * Ixz * Iyz + Ixx * Iyz * Iyz + Ixy * Ixy * Izz - Ixx * Iyy * Izz);
        Lz = (-(Ixz * Iyy * Rx) + Ixy * Iyz * Rx + Ixy * Ixz * Ry - Ixx * Iyz * Ry - Ixy * Ixy * Rz + Ixx * Iyy * Rz) /
             (Ixz * Ixz * Iyy - 2. * Ixy * Ixz * Iyz + Ixy * Ixy * Izz + Ixx * (Iyz * Iyz - Iyy * Izz));

        double S = 0;
        for (size_t j = 0; j < npts; ++j) {
            weights[j] = 1.0 + Lx * Dx[j] + Ly * Dy[j] + Lz * Dz[j];
            S += weights[j];
        }

        for (size_t j = 0; j < npts; ++j) {
            weights[j] /= S;
        }

        // insert the interpolant weights into the global (sparse) interpolant matrix
        for (size_t i = 0; i < npts; ++i) {
            size_t jp = closest[i].payload();
            weights_triplets.emplace_back(WeightMatrix::Triplet(ip, jp, weights[i]));
        }
    }

    // fill-in sparse matrix
    W.setFromTriplets(weights_triplets);
}


void PseudoLaplace::print(std::ostream& out) const {
    out << "PseudoLaplace[";
    MethodWeighted::print(out);
    out << "]";
}


static MethodBuilder<PseudoLaplace> __method("pseudo-laplace");


}  // namespace other
}  // namespace method
}  // namespace mir
