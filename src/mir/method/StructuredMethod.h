/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @date June 2017


#ifndef mir_method_StructuredMethod_h
#define mir_method_StructuredMethod_h

#include "mir/method/MethodWeighted.h"

#include <vector>
#include "eckit/geometry/Point3.h"
#include "atlas/grid.h"
#include "mir/param/MIRParametrisation.h"


namespace atlas {
namespace grid {
class StructuredGrid;
}
}
namespace mir {
namespace param {
class MIRParametrisation;
}
namespace repres {
class Representation;
}
namespace util {
class MIRGrid;
}
}


namespace mir {
namespace method {


class StructuredMethod : public MethodWeighted {
public:

    StructuredMethod(const param::MIRParametrisation&);
    ~StructuredMethod();

protected:

    // Utility types
    typedef eckit::geometry::Point3 point_3d_t;
    typedef std::pair<Latitude, Longitude> point_ll_t;
    typedef std::vector< WeightMatrix::Triplet > triplets_t;

    // Find nearest West-East bounding i indices
    void left_right_lon_indexes(
            Longitude& in,
            const std::vector<point_ll_t>& coords,
            const size_t start,
            const size_t end,
            size_t& left,
            size_t& right) const;


    // Normalize weights triplets such that sum(weights) = 1
    void normalise(triplets_t& triplets) const;

    // Find nearest North-South bounding j indices
    void boundNorthSouth(size_t& jNorth, size_t& jSouth, const double& lat, const std::vector<double>& latitudes) const;

    // Find nearest West-East bounding i indices
    void boundWestEast(size_t& iWest, size_t& iEast, const double& lon, const size_t& Ni, const size_t& iStart) const;

private:

    void assemble(util::MIRStatistics&, WeightMatrix&, const repres::Representation& in, const repres::Representation& out) const;
    virtual void assembleStructuredInput(WeightMatrix&, const repres::Representation& in, const repres::Representation& out) const = 0;

};


}  // namespace method
}  // namespace mir


#endif

