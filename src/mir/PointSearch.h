/*
 * (C) Copyright 1996-2014 ECMWF.
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

#include "eckit/geometry/KPoint.h"
#include "eckit/container/KDMemory.h"
#include "eckit/container/KDTree.h"
#include "eckit/memory/NonCopyable.h"
#include "atlas/grid/Grid.h"

//-----------------------------------------------------------------------------


namespace mir {

//-----------------------------------------------------------------------------

class PointSearch : private eckit::NonCopyable {

    struct TreeTrait {
        typedef eckit::geometry::LLPoint Point;
        typedef size_t                   Payload;
    };

    typedef eckit::KDTreeMemory<TreeTrait> TreeType;

    typedef TreeType::PointType   PointType;
    typedef TreeType::NodeInfo    NodeInfo;
    typedef TreeType::PayloadType PayloadType;
    typedef TreeType::Value       ValueType;

public: // methods

    PointSearch(const std::vector< PointType >& points);

    virtual ~PointSearch();

    /// Finds closts N points to an input point
    void closestNPoints(const PointType& pt, size_t n, std::vector<PointType>& closest, std::vector<PayloadType>& indices);
    void closestNPoints(const PointType& pt, size_t n, std::vector<ValueType>& closest);

protected:
    
    TreeType kd_;

};

} // namespace mir

#endif
