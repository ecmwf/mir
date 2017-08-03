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
#include "mir/repres/Iterator.h"


namespace mir {
namespace param {
class MIRParametrisation;
}
namespace repres {
class Representation;
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
    typedef std::vector< WeightMatrix::Triplet > triplet_vector_t;

    // Find nearest West-East bounding i indices
    void left_right_lon_indexes(
            const Longitude& in,
            const std::vector<repres::Iterator::point_ll_t>& coords,
            const size_t start,
            const size_t end,
            size_t& left,
            size_t& right) const;


    // Normalize weights triplets such that sum(weights) = 1
    void normalise(triplet_vector_t& triplets) const;

    // Get lat/lon point coordinates from representation
    void getRepresentationPoints(const repres::Representation&, std::vector<repres::Iterator::point_ll_t>&, Latitude& minimum, Latitude& maximum) const;

    // Get latitudes list from representation
    void getRepresentationLatitudes(const repres::Representation&, std::vector<Latitude>&) const;

    // Find nearest North-South bounding j indices
    void boundNorthSouth(const Latitude&, const std::vector<Latitude>&, size_t& jNorth, size_t& jSouth) const;

    // Find nearest West-East bounding i indices
    void boundWestEast(const Longitude&, const size_t& Ni, const size_t& iStart, size_t& iWest, size_t& iEast) const;

private:

    void assemble(util::MIRStatistics&, WeightMatrix&, const repres::Representation& in, const repres::Representation& out) const;
    virtual void assembleStructuredInput(WeightMatrix&, const repres::Representation& in, const repres::Representation& out) const = 0;

};


}  // namespace method
}  // namespace mir


#endif

