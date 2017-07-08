/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Tiago Quintino
/// @author Pedro Maciel
/// @date   Apr 2015


#ifndef mir_method_PointSearch_h
#define mir_method_PointSearch_h

#include <vector>
#include "eckit/memory/NonCopyable.h"
#include "eckit/memory/ScopedPtr.h"
#include "atlas/interpolation/method/PointIndex3.h"
#include "mir/util/Compare.h"


namespace mir {
namespace repres {
class Representation;
}
}


namespace mir {
namespace util {


/// Class for fast searches in point clouds following kd-tree algorithms
/// @todo test kd-tree stored in shared memory?
class PointSearch : private eckit::NonCopyable {
private:

    typedef atlas::interpolation::method::PointIndex3           TreeType;
    typedef atlas::interpolation::method::PointIndex3::Point    Point;
    typedef atlas::interpolation::method::PointIndex3::iterator iterator;

public:

    typedef atlas::interpolation::method::PointIndex3::Value PointValueType;
    typedef eckit::geometry::Point3   PointType;

    typedef compare::ACompareFn  <size_t> CompareType;
    typedef compare::IsAnythingFn<size_t> CompareTypeNone;

public:

    PointSearch(const repres::Representation&, const CompareType& =CompareTypeNone());

public:

    /// Finds closest point to an input point
    PointValueType closestPoint(const PointType& pt) const;

    /// Finds closest N points to an input point
    void closestNPoints(const PointType& pt, size_t n, std::vector<PointValueType>& closest) const;

    /// Finds closest points within a radius
    void closestWithinRadius(const PointType& pt, double radius, std::vector<PointValueType>& closest) const;

    void statsPrint(std::ostream& o, bool fancy) const;
    void statsReset() const;

protected:

    eckit::ScopedPtr<TreeType> tree_;

};


}  // namespace util
}  // namespace mir


#endif

