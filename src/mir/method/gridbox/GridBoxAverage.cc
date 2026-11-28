// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/method/gridbox/GridBoxAverage.h"

#include <algorithm>
#include <forward_list>
#include <iterator>
#include <sstream>
#include <utility>
#include <variant>

#include "eckit/geo/Grid.h"
#include "eckit/geo/Search.h"
#include "eckit/types/FloatCompare.h"

#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Representation.h"
#include "mir/util/Domain.h"
#include "mir/util/Exceptions.h"
#include "mir/util/GridBox.h"
#include "mir/util/Log.h"
#include "mir/util/LonLatToXYZ.h"
#include "mir/util/Search.h"
#include "mir/util/Trace.h"


namespace mir::method::gridbox {


static const MethodBuilder<GridBoxAverage> __builder("grid-box-average");


GridBoxAverage::GridBoxAverage(const param::MIRParametrisation& parametrisation) : GridBoxMethod(parametrisation) {
    if (parametrisation.userParametrisation().has("rotation") ||
        parametrisation.fieldParametrisation().has("rotation")) {
        throw exception::UserError("GridBoxMethod: rotated input/output not supported");
    }
}


void GridBoxAverage::assemble(util::MIRStatistics& /*unused*/, WeightMatrix& W, const repres::Representation& in,
                              const repres::Representation& out) const {
    auto& log = Log::debug();
    log << "GridBoxMethod::assemble"
           "\ninput: "
        << in << "\ndomain: " << in.domain() << "\noutput: " << out << "\ndomain: " << out.domain() << std::endl;

    if (!in.domain().contains(out.domain())) {
        std::ostringstream msg;
        msg << "GridBoxMethod: input domain must contain output domain";
        throw exception::UserError(msg.str());
    }

    const Log::Plural gridBoxes("grid box", "grid boxes");
    log << "GridBoxMethod: intersect " << Log::Pretty(out.numberOfPoints()) << " from "
        << Log::Pretty(in.numberOfPoints(), gridBoxes) << std::endl;


    // init structure used to fill in sparse matrix
    // TODO: triplets, really? why not writing to the matrix directly?
    std::vector<WeightMatrix::Triplet> weights_triplets;
    std::vector<WeightMatrix::Triplet> triplets;
    eckit::geo::Search::Neighbours closest;


    // set input and output grid boxes
    struct GridBoxes : std::vector<util::GridBox> {
        explicit GridBoxes(const repres::Representation& rep) : vector(rep.gridBoxes()) {
            ASSERT(size() == rep.numberOfPoints());
        }

        double getLongestGridBoxDiagonal() const {
            double R = 0.;
            for (const auto& box : *this) {
                R = std::max(R, box.diagonal());
            }
            ASSERT(R > 0.);
            return R;
        }
    };

    const GridBoxes inBoxes(in);
    const GridBoxes outBoxes(out);
    const auto R = inBoxes.getLongestGridBoxDiagonal() + outBoxes.getLongestGridBoxDiagonal();

    const util::LonLatToXYZ point3(in.canonicalGrid().figure(), poleDisplacement(in));

    size_t nbFailures = 0;
    std::forward_list<std::pair<size_t, PointLonLat>> failures;


    // set input k-d tree for grid boxes indices
    std::unique_ptr<const eckit::geo::Search> tree;
    {
        trace::ResourceUsage usage("GridBoxMethod::assemble create k-d tree");
        tree = util::make_search(parametrisation_, in);
    }

    {
        trace::ProgressTimer progress("Intersecting", outBoxes.size(), gridBoxes);

        const auto& points = out.canonicalGrid();
        for (auto it = points.cbegin(); it; ++it) {
            if (++progress) {
                log << *tree << std::endl;
            }

            const auto p = std::get<PointLonLat>(*it);

            // lookup
            closest = tree->search_radius(point3(p), R);
            ASSERT(!closest.empty());


            // calculate grid box intersections
            triplets.clear();
            triplets.reserve(closest.size());

            auto i          = it.index();
            const auto& box = outBoxes.at(i);
            double area     = box.area();
            ASSERT(area > 0.);

            double sumSmallAreas = 0.;
            bool areaMatch       = false;
            for (auto& c : closest) {
                auto j        = c.index;
                auto smallBox = inBoxes.at(j);

                if (box.intersects(smallBox)) {
                    double smallArea = smallBox.area();
                    ASSERT(smallArea > 0.);

                    // weight clipped as numerical conditioning for intersection
                    triplets.emplace_back(i, j, std::min(1., smallArea / area));
                    sumSmallAreas += smallArea;

                    if ((areaMatch = eckit::types::is_approximately_equal(area, sumSmallAreas, 1. /*m^2*/))) {
                        break;
                    }
                }
            }


            // insert the interpolant weights into the global (sparse) interpolant matrix
            if (areaMatch) {
                std::copy(triplets.begin(), triplets.end(), std::back_inserter(weights_triplets));
            }
            else {
                ++nbFailures;
                failures.emplace_front(i, p);
            }
        }
    }
    log << "Intersected " << Log::Pretty(weights_triplets.size(), gridBoxes) << std::endl;

    if (nbFailures > 0) {
        auto& warning = Log::warning();
        warning << "Failed to intersect " << Log::Pretty(nbFailures, gridBoxes) << ":";
        size_t count = 0;
        for (const auto& f : failures) {
            warning << "\n\tpoint " << f.first << " " << f.second;
            if (++count > 10) {
                warning << "\n\t...";
                break;
            }
        }
        warning << std::endl;
    }


    // fill sparse matrix
    ASSERT_NONEMPTY_INTERPOLATION("GridBoxMethod", !weights_triplets.empty());
    W.setFromTriplets(weights_triplets);
}


const char* GridBoxAverage::type() const {
    return "grid-box-average";
}


int GridBoxAverage::version() const {
    return 0;
}


}  // namespace mir::method::gridbox
