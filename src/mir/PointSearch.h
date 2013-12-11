/*
 * (C) Copyright 1996-2013 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Peter Bispham
/// @author Tiago Quintino
/// @date Oct 2013

#ifndef eckit_grid_PointSearch_H
#define eckit_grid_PointSearch_H

#include "eckit/container/KDPoint.h"
#include "eckit/container/KDTree.h"
#include "eckit/memory/NonCopyable.h"
#include "eckit/grid/Grid.h"

//-----------------------------------------------------------------------------


namespace mir {

//-----------------------------------------------------------------------------

typedef eckit::KDPoint<unsigned int> IndexPoint;

class PointSearch : private eckit::NonCopyable {

public: // methods

    PointSearch(const std::vector<eckit::grid::Point2D>& points);

    virtual ~PointSearch();

    /// Finds closts N points to an input point
    void closestNPoints(const eckit::grid::Point2D& pt, size_t n, std::vector<eckit::grid::Point2D>& closest, std::vector<unsigned int>& indices);


protected:
    
    eckit::KDTree<IndexPoint> kd_;


};

} // namespace mir

#endif
