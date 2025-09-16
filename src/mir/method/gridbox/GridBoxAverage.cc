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


#include "mir/method/gridbox/GridBoxAverage.h"

#include <algorithm>
#include <forward_list>
#include <iterator>
#include <sstream>
#include <utility>

#include "eckit/types/FloatCompare.h"

#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Iterator.h"
#include "mir/repres/Representation.h"
#include "mir/search/PointSearch.h"
#include "mir/util/Domain.h"
#include "mir/util/Exceptions.h"
#include "mir/util/GridBox.h"
#include "mir/util/Log.h"
#include "mir/util/Point2ToPoint3.h"
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
    std::vector<search::PointSearch::PointValueType> closest;


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

    util::Point2ToPoint3 point3(in, poleDisplacement());

    size_t nbFailures = 0;
    std::forward_list<std::pair<size_t, PointLatLon>> failures;


    // set input k-d tree for grid boxes indices
    std::unique_ptr<search::PointSearch> tree;
    {
        trace::ResourceUsage usage("GridBoxMethod::assemble create k-d tree");
        tree = std::make_unique<search::PointSearch>(parametrisation_, in);
    }

    {
        trace::ProgressTimer progress("Intersecting", outBoxes.size(), gridBoxes);

        for (const std::unique_ptr<repres::Iterator> it(out.iterator()); it->next();) {
            if (++progress) {
                log << *tree << std::endl;
            }


            // lookup
            tree->closestWithinRadius(point3(*(*it)), R, closest);
            ASSERT(!closest.empty());


            // calculate grid box intersections
            triplets.clear();
            triplets.reserve(closest.size());

            auto i          = it->index();
            const auto& box = outBoxes.at(i);
            double area     = box.area();
            ASSERT(area > 0.);

            double sumSmallAreas = 0.;
            bool areaMatch       = false;
            for (auto& c : closest) {
                auto j        = c.payload();
                auto smallBox = inBoxes.at(j);

                if (box.intersects(smallBox)) {
                    double smallArea = smallBox.area();
                    ASSERT(smallArea > 0.);

                    triplets.emplace_back(i, j, smallArea / area);
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
                failures.emplace_front(i, it->pointUnrotated());
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
    return 5;
}


}  // namespace mir::method::gridbox
