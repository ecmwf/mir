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


namespace mir {
namespace util {


/// Class for fast searches in point clouds following kd-tree algorithms
/// @todo test kd-tree stored in shared memory ?

class PointSearch : private eckit::NonCopyable {
  typedef atlas::PointIndex3 TreeType;

  typedef atlas::PointIndex3::Point Point;
  typedef atlas::PointIndex3::Value ValueType;

  typedef atlas::PointIndex3::iterator iterator;
  typedef eckit::geometry::Point3 PointType;

public:
  PointSearch(const std::vector<Point>& ipts);

  PointSearch(const atlas::Mesh& mesh);

public:  // methods
  /// Finds closest N points to an input point
  void closestNPoints(const PointType& pt, size_t n, std::vector<ValueType>& closest);

protected:
  eckit::ScopedPtr<TreeType> tree_;

private:
  void init(const std::vector<PointType>& points);

  void init(const atlas::Mesh& mesh);

  mutable atlas::Grid::uid_t uid_;
};

}  // namespace util
}  // namespace mir
#endif

