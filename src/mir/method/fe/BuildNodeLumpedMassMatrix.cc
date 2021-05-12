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


#include "mir/method/fe/BuildNodeLumpedMassMatrix.h"

#include <utility>

#include "eckit/types/FloatCompare.h"

#include "mir/util/Atlas.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Types.h"


namespace mir {
namespace method {
namespace fe {


BuildNodeLumpedMassMatrix::BuildNodeLumpedMassMatrix(std::string name, bool force_recompute) :
    name_(std::move(name)), force_recompute_(force_recompute) {}


atlas::Field& BuildNodeLumpedMassMatrix::operator()(atlas::Mesh& mesh) const {
    using namespace atlas;

    bool recompute = force_recompute_ || !mesh.nodes().has_field(name_);

    if (recompute) {
        ATLAS_TRACE("BuildNodeLumpedMassMatrix");
        ASSERT(mesh.generated());

        auto& nodes    = mesh.nodes();
        auto coords    = array::make_view<double, 2>(nodes.field("xyz"));
        auto nbRealPts = nodes.metadata().has("NbRealPts") ? nodes.metadata().get<idx_t>("NbRealPts") : nodes.size();

        if (!nodes.has_field(name_)) {
            nodes.add(Field(name_, array::make_datatype<double>(), array::make_shape(nodes.size())));
        }

        auto mass = array::make_view<double, 1>(nodes.field(name_));
        ASSERT(0 < nbRealPts && nbRealPts <= idx_t(mass.size()));
        mass.assign(0.);


        // North/South pole points
        std::vector<bool> northPole(size_t(nodes.size()), false);
        std::vector<bool> southPole(size_t(nodes.size()), false);

        for (idx_t n = 0; n < nodes.size(); ++n) {
            if (eckit::types::is_approximately_equal(0., coords(n, 0)) &&
                eckit::types::is_approximately_equal(0., coords(n, 1))) {
                (coords(n, 2) > 0 ? northPole : southPole)[size_t(n)] = true;
            }
        }


        // Nodal distributions (except to pole points)
        // assumes:
        // - nb_cols == 3 implies triangle
        // - nb_cols == 4 implies quadrilater%al
        // - no other element is supported at the time
        const auto& connectivity = mesh.cells().node_connectivity();

        for (idx_t e = 0; e < connectivity.rows(); ++e) {
            auto nb_cols = size_t(connectivity.cols(e));
            ASSERT(nb_cols == 3 || nb_cols == 4);

            std::vector<idx_t> idx(nb_cols);
            for (size_t n = 0; n < nb_cols; ++n) {
                idx[size_t(n)] = connectivity(e, idx_t(n));
            }

            static const double oneThird  = 1. / 3.;
            static const double oneFourth = 1. / 4.;

            double nodalDistribution =
                nb_cols == 3 ? oneThird * interpolation::element::Triag3D(
                                              PointXYZ{coords(idx[0], 0), coords(idx[0], 1), coords(idx[0], 2)},
                                              PointXYZ{coords(idx[1], 0), coords(idx[1], 1), coords(idx[1], 2)},
                                              PointXYZ{coords(idx[2], 0), coords(idx[2], 1), coords(idx[2], 2)})
                                              .area()
                             : oneFourth * interpolation::element::Quad3D(
                                               PointXYZ{coords(idx[0], 0), coords(idx[0], 1), coords(idx[0], 2)},
                                               PointXYZ{coords(idx[1], 0), coords(idx[1], 1), coords(idx[1], 2)},
                                               PointXYZ{coords(idx[2], 0), coords(idx[2], 1), coords(idx[2], 2)},
                                               PointXYZ{coords(idx[3], 0), coords(idx[3], 1), coords(idx[3], 2)})
                                               .area();

            ASSERT(nodalDistribution > 0.);
            for (auto i : idx) {
                if (i < nbRealPts) {
                    mass[i] += nodalDistribution;
                }
            }
        }


        // North/South pole nodal re-distribution
        auto poleNodalDistribution = [&mass, nbRealPts](std::vector<bool>& pole) {
            ASSERT(0 < nbRealPts && nbRealPts <= idx_t(pole.size()));

            double contribution = 0.;
            size_t nb           = 0;
            for (idx_t i = 0; i < nbRealPts; ++i) {
                if (pole[size_t(i)]) {
                    contribution += mass[i];
                    nb++;
                }
            }

            if (nb > 0) {
                auto nodalDistribution = contribution / double(nb);
                for (idx_t i = 0; i < nbRealPts; ++i) {
                    if (pole[size_t(i)]) {
                        mass[i] = nodalDistribution;
                    }
                }
            }
        };
        poleNodalDistribution(northPole);
        poleNodalDistribution(southPole);
    }

    return mesh.nodes().field(name_);
}


}  // namespace fe
}  // namespace method
}  // namespace mir
