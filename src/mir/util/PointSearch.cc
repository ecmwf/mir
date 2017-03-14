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

#include <vector>
#include <limits>

#include "mir/util/PointSearch.h"

#include "mir/method/GridSpace.h"

namespace mir {
namespace util {

//----------------------------------------------------------------------------------------------------------------------

PointSearch::PointSearch(const std::vector<PointType>& points) {
    init(points);
}


PointSearch::PointSearch(const mir::method::GridSpace& sp, const CompareType& isok) {
    init(sp,isok);
}


void PointSearch::statsPrint(std::ostream& s, bool fancy) const {
    tree_->statsPrint(s, fancy);
}

void PointSearch::statsReset() const {
    tree_->statsReset();
}

PointSearch::PointValueType PointSearch::closestPoint(const PointSearch::PointType& pt) const {
    const atlas::interpolation::method::PointIndex3::NodeInfo nn = tree_->nearestNeighbour(pt);
    return nn.value();
}


void PointSearch::closestNPoints(const PointType& pt, size_t n, std::vector<PointValueType>& closest) const {
    using atlas::interpolation::method::PointIndex3;

    // Small optimisation
    if(n == 1) {
        closest.clear();
        closest.push_back(closestPoint(pt));
        return;
    }

    PointIndex3::NodeList nn = tree_->kNearestNeighbours(pt, n);

    closest.clear();
    closest.reserve(n);
    for (PointIndex3::NodeList::iterator it = nn.begin(); it != nn.end(); ++it)
        closest.push_back(it->value());
}


void PointSearch::closestWithinRadius(const PointType& pt, double radius, std::vector<PointValueType>& closest) const {
    using atlas::interpolation::method::PointIndex3;

    PointIndex3::NodeList r = tree_->findInSphere(pt,radius);

    closest.clear();
    closest.reserve(r.size());
    for (PointIndex3::NodeList::iterator it = r.begin(); it != r.end(); ++it)
        closest.push_back(it->value());
}


void PointSearch::init(const std::vector<PointType>& points) {

    using atlas::interpolation::method::PointIndex3;

    std::vector<PointIndex3::Value> pidx;
    pidx.reserve(points.size());

    for (size_t ip = 0; ip < points.size(); ++ip)
        pidx.push_back(PointIndex3::Value(PointIndex3::Point(points[ip]), ip));

    tree_.reset(new PointIndex3());
    tree_->build(pidx.begin(), pidx.end());
}


void PointSearch::init(const method::GridSpace& sp, const CompareType& isok) {

    const size_t npts = sp.grid().npts();
    ASSERT(npts > 0);

    const double infty = std::numeric_limits< double >::infinity();
    const PointType farpoint(infty,infty,infty);

    std::vector<PointType> points;
    points.reserve(npts);

    atlas::array::ArrayView<double, 2> coords = sp.coordsXYZ();
    for (size_t ip = 0; ip < npts; ++ip) {
        points.push_back(isok(ip) ? PointType(coords[ip].data()) : farpoint );
    }

    init(points);
}

//----------------------------------------------------------------------------------------------------------------------

}  // namespace util
}  // namespace mir
