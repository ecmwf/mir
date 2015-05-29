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

#include <vector>


namespace mir {
namespace util {


PointSearch::PointSearch(const std::vector<PointType>& points) {
    init(points);
}


PointSearch::PointSearch(const atlas::Mesh& mesh, const CompareType& isok) {
    init(mesh,isok);
}


void PointSearch::closestNPoints(const PointType& pt, size_t n, std::vector<ValueType>& closest) {
    using atlas::PointIndex3;

    PointIndex3::NodeList nn = tree_->kNearestNeighbours(pt, n);

    closest.clear();
    closest.reserve(n);
    for (PointIndex3::NodeList::iterator it = nn.begin(); it != nn.end(); ++it)
        closest.push_back(it->value());
}


void PointSearch::closestWithinRadius(const PointType& pt, double radius, std::vector<ValueType>& closest) {
    using atlas::PointIndex3;

    PointIndex3::NodeList r = tree_->findInSphere(pt,radius);

    closest.clear();
    closest.reserve(r.size());
    for (PointIndex3::NodeList::iterator it = r.begin(); it != r.end(); ++it)
        closest.push_back(it->value());
}


void PointSearch::init(const std::vector<PointType>& points) {
    using atlas::PointIndex3;

    std::vector<PointIndex3::Value> pidx;
    pidx.reserve(points.size());

    for (size_t ip = 0; ip < points.size(); ++ip)
        pidx.push_back(PointIndex3::Value(PointIndex3::Point(points[ip]), ip));

    tree_.reset(new PointIndex3());
    tree_->build(pidx.begin(), pidx.end());
}


void PointSearch::init(const atlas::Mesh& mesh, const CompareType& isok) {
    ASSERT(mesh.has_function_space("nodes"));

    atlas::FunctionSpace& nodes = mesh.function_space("nodes");
    ASSERT(nodes.has_field("xyz"));

    size_t npts = nodes.shape(0);
    ASSERT(npts > 0);

    std::vector<PointType> points;
    points.reserve(npts);

    atlas::ArrayView<double, 2> coords(nodes.field<double>("xyz"));
    for (size_t ip = 0; ip < npts; ++ip)
        if (isok(ip))
            points.push_back(coords[ip].data());

    init(points);
}


}  // namespace util
}  // namespace mir
