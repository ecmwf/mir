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


#include "mir/method/gridbox/GridBoxStatistics.h"

#include <algorithm>
#include <forward_list>
#include <ostream>
#include <sstream>
#include <utility>

#include "mir/method/gridbox/GridBoxMethod.h"
#include "mir/method/solver/Statistics.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Representation.h"
#include "mir/search/PointSearch.h"
#include "mir/stats/Field.h"
#include "mir/util/Domain.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"
#include "mir/util/Point2ToPoint3.h"
#include "mir/util/Trace.h"


namespace mir::method::gridbox {


static const MethodBuilder<GridBoxStatistics> __builder("grid-box-statistics");


GridBoxStatistics::GridBoxStatistics(const param::MIRParametrisation& param) : GridBoxMethod(param) {
    std::string stats = "maximum";
    param.get("interpolation-statistics", stats);

    setSolver(new solver::Statistics(param, stats::FieldFactory::build(stats, param)));
}


void GridBoxStatistics::assemble(util::MIRStatistics& /*unused*/, WeightMatrix& W, const repres::Representation& in,
                                 const repres::Representation& out) const {
    auto& log = Log::debug();
    log << "GridBoxStatistics::assemble (input: " << in << ", output: " << out << ")" << std::endl;


    if (!in.domain().contains(out.domain())) {
        std::ostringstream msg;
        msg << "GridBoxStatistics: input must contain output (input:" << in.domain() << ", output:" << out.domain()
            << ")";
        throw exception::UserError(msg.str());
    }

    const Log::Plural boxes("grid box", "grid boxes");
    const Log::Plural points("point");
    log << "GridBoxStatistics: intersect " << Log::Pretty(out.numberOfPoints()) << " from "
        << Log::Pretty(in.numberOfPoints(), boxes) << std::endl;


    // init structure used to fill in sparse matrix
    // TODO: triplets, really? why not writing to the matrix directly?
    std::vector<WeightMatrix::Triplet> weights_triplets;
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

    const GridBoxes outBoxes(out);

    util::Point2ToPoint3 point3(in, poleDisplacement());

    size_t nbFailures = 0;
    std::forward_list<std::pair<size_t, PointLatLon>> failures;


    // set input k-d tree for grid boxes indices
    std::unique_ptr<search::PointSearch> tree;
    {
        trace::ResourceUsage usage("GridBoxStatistics::assemble create k-d tree");
        tree = std::make_unique<search::PointSearch>(parametrisation_, in);
    }

    {
        trace::ProgressTimer progress("Containing", outBoxes.size(), boxes);

        const auto R = outBoxes.getLongestGridBoxDiagonal();

        auto point_2D = [](const Point3& point) -> Point2 {
            atlas::PointLonLat pll;
            util::Earth::convertCartesianToSpherical(point, pll);

            // notice the order
            return {pll[1], pll[0]};
        };

        for (size_t i = 0; i < outBoxes.size(); ++i) {
            if (++progress) {
                log << *tree << std::endl;
            }


            // lookup
            const auto& box = outBoxes[i];
            tree->closestWithinRadius(point3(box.centre()), R, closest);
            if (closest.empty()) {
                continue;
            }


            // calculate grid box contains
            std::vector<size_t> js;
            for (auto& c : closest) {
                if (box.contains(point_2D(c.point()))) {
                    js.emplace_back(c.payload());
                }
            }


            // insert the interpolant weights into the global (sparse) interpolant matrix
            if (!js.empty()) {
                std::sort(js.begin(), js.end());
                const auto weight = 1. / static_cast<double>(js.size());
                for (auto j : js) {
                    weights_triplets.emplace_back(i, j, weight);
                }
            }
            else {
                ++nbFailures;
            }
        }
        log << "Contained " << Log::Pretty(weights_triplets.size(), points) << " in "
            << Log::Pretty(outBoxes.size(), boxes) << std::endl;
    }

    if (nbFailures > 0) {
        auto& warning = Log::warning();
        warning << "Failed to contain " << Log::Pretty(nbFailures, points);
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
    ASSERT_NONEMPTY_INTERPOLATION("GridBoxStatistics", !weights_triplets.empty());
    W.setFromTriplets(weights_triplets);
}


const char* GridBoxStatistics::name() const {
    return "grid-box-statistics";
}


int GridBoxStatistics::version() const {
    return 5;
}


}  // namespace mir::method::gridbox
