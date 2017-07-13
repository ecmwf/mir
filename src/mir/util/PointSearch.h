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
#include "eckit/container/sptree/SPValue.h"
#include "eckit/container/sptree/SPNodeInfo.h"


namespace mir {
namespace repres {
class Representation;
}
}


namespace mir {
namespace util {


class PointSearchTree {
public:
    typedef eckit::geometry::Point3             Point;
    typedef size_t                              Payload;
    typedef eckit::SPValue<PointSearchTree>     PointValueType;

public:
    virtual ~PointSearchTree() {};

    virtual void build(std::vector<PointValueType>&) = 0;

    virtual void insert(const PointValueType&) = 0;
    virtual void statsPrint(std::ostream&, bool) = 0;
    virtual void statsReset() = 0;

    virtual PointValueType nearestNeighbour(const Point&) = 0;
    virtual std::vector<PointValueType> kNearestNeighbours(const Point&, size_t k) = 0;
    virtual std::vector<PointValueType> findInSphere(const Point&, double) = 0;

    virtual bool ready() const = 0;
    virtual void commit() = 0;
    virtual void print(std::ostream &) const = 0;

     friend std::ostream &operator<<(std::ostream &s, const PointSearchTree &p) {
        p.print(s);
        return s;
    }
};


/// Class for fast searches in point clouds following kd-tree algorithms
/// @todo test kd-tree stored in shared memory?
class PointSearch : private eckit::NonCopyable {
public:

    typedef PointSearchTree::Payload        ValueType;
    typedef PointSearchTree::Point          PointType;
    typedef PointSearchTree::PointValueType PointValueType;

    typedef compare::ACompareFn  <size_t> CompareType;
    typedef compare::IsAnythingFn<size_t> CompareTypeNone;

public:

    PointSearch(const repres::Representation&, const CompareType& = CompareTypeNone());

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

    eckit::ScopedPtr<PointSearchTree> tree_;

private:

    void build(const repres::Representation& r, const CompareType&);


};


}  // namespace util
}  // namespace mir


#endif

