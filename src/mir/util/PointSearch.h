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


#ifndef mir_method_PointSearch_H
#define mir_method_PointSearch_H

#include "eckit/memory/NonCopyable.h"
#include "eckit/memory/ScopedPtr.h"
#include "atlas/Grid.h"
#include "atlas/PointIndex3.h"
#include "mir/util/Compare.h"


namespace mir {
namespace util {


/// Class for fast searches in point clouds following kd-tree algorithms
/// @todo test kd-tree stored in shared memory?
class PointSearch : private eckit::NonCopyable {
private:

    typedef atlas::PointIndex3           TreeType;
    typedef atlas::PointIndex3::Point    Point;
    typedef atlas::PointIndex3::iterator iterator;

public:

    typedef atlas::PointIndex3::Value PointValueType;
    typedef eckit::geometry::Point3   PointType;

    typedef compare::compare_fn     <size_t> CompareType;
    typedef compare::is_anything_fn <size_t> CompareTypeNone;

public:

    PointSearch(const std::vector<Point>& ipts);

    PointSearch(const atlas::Mesh& mesh, const CompareType& isok=CompareTypeNone());

public:

    /// Finds closest point to an input point
    PointValueType closestPoint(const PointType& pt);

    /// Finds closest N points to an input point
    void closestNPoints(const PointType& pt, size_t n, std::vector<PointValueType>& closest);

    /// Finds closest points within a radius
    void closestWithinRadius(const PointType& pt, double radius, std::vector<PointValueType>& closest);

protected:

    eckit::ScopedPtr<TreeType> tree_;

private:

    void init(const std::vector<PointType>& points);

    void init(const atlas::Mesh& mesh, const CompareType& isok=CompareTypeNone());

    mutable atlas::Grid::uid_t uid_;

};


}  // namespace util
}  // namespace mir
#endif

