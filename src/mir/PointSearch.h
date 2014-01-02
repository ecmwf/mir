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

#ifndef mir_PointSearch_H
#define mir_PointSearch_H

#include "eckit/container/sptree/SPPoint.h"
#include "eckit/container/KDMemory.h"
#include "eckit/container/KDTree.h"
#include "eckit/memory/NonCopyable.h"
#include "eckit/grid/Grid.h"

//-----------------------------------------------------------------------------


namespace mir {

//-----------------------------------------------------------------------------

class PointSearch : private eckit::NonCopyable {

    struct TreeTrait {
        typedef eckit::KDMemory   Alloc;
        typedef eckit::SPPoint<2> Point;
        typedef unsigned int      Payload;
    };

    typedef eckit::KDTree<TreeTrait> TreeType;
    typedef eckit::KDTree<TreeTrait>::PointType PointType;
    typedef eckit::KDTree<TreeTrait>::NodeInfo  NodeInfo;
    typedef eckit::KDTree<TreeTrait>::PayloadType PayloadType;

public: // methods

    PointSearch(const std::vector<eckit::grid::Point2D>& points);

    virtual ~PointSearch();

    /// Finds closts N points to an input point
    void closestNPoints(const eckit::grid::Point2D& pt, size_t n, std::vector<eckit::grid::Point2D>& closest, std::vector<unsigned int>& indices);


protected:
    
    TreeType kd_;

};

} // namespace mir

#endif
