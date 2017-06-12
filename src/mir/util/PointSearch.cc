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
#include "mir/util/MIRGrid.h"


namespace mir {
namespace util {


PointSearch::PointSearch(const std::vector<PointType>& points) {
    init(points);
}


PointSearch::PointSearch(const MIRGrid& sp, const CompareType& isok) {
    init(sp, isok);
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
    if(n == 1) {
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
    TreeType::NodeList r = tree_->findInSphere(pt,radius);

    closest.clear();
    closest.reserve(r.size());
    for (TreeType::NodeList::iterator it = r.begin(); it != r.end(); ++it) {
        closest.push_back(it->value());
    }
}


void PointSearch::init(const std::vector<PointType>& points) {

    std::vector<PointValueType> pidx;
    pidx.reserve(points.size());

    for (size_t ip = 0; ip < points.size(); ++ip) {
        pidx.push_back(PointValueType(TreeType::Point(points[ip]), ip));
    }

    tree_.reset(new TreeType());
    tree_->build(pidx.begin(), pidx.end());
}


void PointSearch::init(const MIRGrid& sp, const CompareType& isok) {

    const size_t npts = sp.size();
    ASSERT(npts > 0);

    const double infty = std::numeric_limits< double >::infinity();
    const PointType farpoint(infty,infty,infty);

    std::vector<PointType> points;
    points.reserve(npts);

    atlas::array::ArrayView<double, 2> coords = atlas::array::make_view< double, 2 >(sp.coordsXYZ());
    for (size_t ip = 0; ip < npts; ++ip) {
        points.push_back(isok(ip) ? PointType(coords[ip].data()) : farpoint );
    }

    init(points);
}


}  // namespace util
}  // namespace mir
