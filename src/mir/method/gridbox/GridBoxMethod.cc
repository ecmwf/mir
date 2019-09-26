/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/method/gridbox/GridBoxMethod.h"

#include <algorithm>
#include <sstream>
#include <vector>

#include "eckit/exception/Exceptions.h"
#include "eckit/log/Log.h"
#include "eckit/log/ProgressTimer.h"
#include "eckit/log/ResourceUsage.h"
#include "eckit/log/TraceTimer.h"
#include "eckit/types/FloatCompare.h"
#include "eckit/utils/MD5.h"

#include "mir/config/LibMir.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Representation.h"
#include "mir/search/PointSearch.h"
#include "mir/util/Domain.h"
#include "mir/util/GridBox.h"
#include "mir/util/Pretty.h"


namespace mir {
namespace method {
namespace gridbox {


GridBoxMethod::GridBoxMethod(const param::MIRParametrisation& parametrisation) : MethodWeighted(parametrisation) {
    if (parametrisation.userParametrisation().has("rotation") ||
        parametrisation.fieldParametrisation().has("rotation")) {
        throw eckit::UserError("GridBoxMethod: rotated input/output not supported");
    }
}


GridBoxMethod::~GridBoxMethod() = default;


bool GridBoxMethod::sameAs(const Method& other) const {
    auto o = dynamic_cast<const GridBoxMethod*>(&other);
    return o && name() == o->name() && MethodWeighted::sameAs(*o);
}


void GridBoxMethod::assemble(util::MIRStatistics&, WeightMatrix& W, const repres::Representation& in,
                             const repres::Representation& out) const {
    eckit::Channel& log = eckit::Log::debug<LibMir>();
    log << "GridBoxMethod::assemble (input: " << in << ", output: " << out << ")" << std::endl;


    if (!in.domain().contains(out.domain())) {
        std::ostringstream msg;
        msg << "GridBoxMethod: input must contain output (input:" << in.domain() << ", output:" << out.domain() << ")";
        throw eckit::UserError(msg.str());
    }


    log << "GridBoxMethod: intersect " << util::Pretty(out.numberOfPoints()) << " from "
        << util::Pretty(in.numberOfPoints(), "grid box", "grid boxes") << std::endl;


    // init structure used to fill in sparse matrix
    // TODO: triplets, really? why not writing to the matrix directly?
    std::vector<WeightMatrix::Triplet> weights_triplets;
    std::vector<WeightMatrix::Triplet> triplets;
    std::vector<search::PointSearch::PointValueType> closest;


    // set input and output grid boxes
    struct GridBoxes : std::vector<util::GridBox> {
        GridBoxes(const repres::Representation& rep) : std::vector<util::GridBox>(rep.gridBoxes()) {
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


    // set input k-d tree for grid boxes indices
    std::unique_ptr<search::PointSearch> tree;
    {
        eckit::ResourceUsage usage("GridBoxMethod::assemble create k-d tree", log);
        eckit::TraceTimer<LibMir> timer("k-d tree: create");
        tree.reset(new search::PointSearch(parametrisation_, in));
    }

    {
        eckit::ProgressTimer progress("Intersecting", outBoxes.size(), "grid box", double(5), log);

        const std::unique_ptr<repres::Iterator> it(out.iterator());
        size_t i = 0;
        while (it->next()) {
            if (++progress) {
                log << *tree << std::endl;
            }


            // lookup
            tree->closestWithinRadius(it->point3D(), R, closest);
            ASSERT(!closest.empty());


            // calculate grid box intersections
            triplets.clear();
            triplets.reserve(closest.size());

            double sumIntersectedArea = 0.;
            for (auto c : closest) {
                auto j    = c.payload();
                auto box  = inBoxes.at(j);
                auto area = box.area();

                if (outBoxes.at(i).intersects(box)) {
                    double intersection = box.area();
                    ASSERT(intersection > 0.);
                    ASSERT(area > 0.);
                    triplets.emplace_back(WeightMatrix::Triplet(i, j, intersection / area));
                    sumIntersectedArea += intersection;
                }
            }
            ASSERT(eckit::types::is_approximately_equal(outBoxes.at(i).area(), sumIntersectedArea, 1. /*m2*/));


            // insert the interpolant weights into the global (sparse) interpolant matrix
            ASSERT(!triplets.empty());
            std::copy(triplets.begin(), triplets.end(), std::back_inserter(weights_triplets));


            ++i;
        }
    }
    log << "Intersected " << util::Pretty(triplets.size(), "grid box", "grid boxes") << std::endl;


    // fill sparse matrix
    W.setFromTriplets(weights_triplets);
}


void GridBoxMethod::hash(eckit::MD5& md5) const {
    MethodWeighted::hash(md5);
    md5.add(name());
}


void GridBoxMethod::print(std::ostream& out) const {
    out << "GridBoxMethod["
        << "name=" << name() << ",";
    MethodWeighted::print(out);
    out << "]";
}


bool GridBoxMethod::validateMatrixWeights() const {
    return false;
}


const char* GridBoxMethod::name() const {
    return "grid-box";
}


}  // namespace gridbox
}  // namespace method
}  // namespace mir
