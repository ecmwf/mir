/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Peter Bispham
/// @author Tiago Quintino
/// @author Pedro Maciel
/// @date Apr 2015


#include "mir/util/PointSearch.h"

#include <limits>
#include "mir/repres/Representation.h"
#include "eckit/config/Resource.h"



namespace mir {
namespace util {



PointSearch::PointSearch(const repres::Representation& r, const CompareType& isok) {
    const size_t npts = r.numberOfPoints();
    ASSERT(npts > 0);

    const double infty = std::numeric_limits< double >::infinity();
    const PointType farpoint(infty, infty, infty);

    tree_.reset(new TreeType());

    static bool fastBuildKDTrees = eckit::Resource<bool>("$ATLAS_FAST_BUILD_KDTREES", true); // We use the same Resource as ATLAS for now

    if (fastBuildKDTrees) {
        std::vector<PointValueType> points;
        points.reserve(npts);

        const eckit::ScopedPtr<repres::Iterator> it(r.iterator());
        size_t i = 0;
        while (it->next()) {
            ASSERT(i < npts);
            points.push_back(PointValueType(isok(i) ? PointType(it->point3D()) : farpoint, i));
            ++i;
        }

        tree_->build(points.begin(), points.end());
    }
    else {
        const eckit::ScopedPtr<repres::Iterator> it(r.iterator());
        size_t i = 0;
        while (it->next()) {
            ASSERT(i < npts);
            tree_->insert(PointValueType(isok(i) ? PointType(it->point3D()) : farpoint, i));
            ++i;
        }
    }
}


void PointSearch::statsPrint(std::ostream& s, bool fancy) const {
    tree_->statsPrint(s, fancy);
}


void PointSearch::statsReset() const {
    tree_->statsReset();
}


PointSearch::PointValueType PointSearch::closestPoint(const PointSearch::PointType& pt) const {
    const TreeType::NodeInfo nn = tree_->nearestNeighbour(pt);

    return nn.value();
}


void PointSearch::closestNPoints(const PointType& pt, size_t n, std::vector<PointValueType>& closest) const {

    // Small optimisation
    if (n == 1) {
        closest.clear();
        closest.push_back(closestPoint(pt));
        return;
    }

    TreeType::NodeList nn = tree_->kNearestNeighbours(pt, n);

    closest.clear();
    closest.reserve(n);
    for (TreeType::NodeList::iterator it = nn.begin(); it != nn.end(); ++it) {
        closest.push_back(it->value());
    }
}


void PointSearch::closestWithinRadius(const PointType& pt, double radius, std::vector<PointValueType>& closest) const {
    TreeType::NodeList r = tree_->findInSphere(pt, radius);

    closest.clear();
    closest.reserve(r.size());
    for (TreeType::NodeList::iterator it = r.begin(); it != r.end(); ++it) {
        closest.push_back(it->value());
    }
}





}  // namespace util
}  // namespace mir
