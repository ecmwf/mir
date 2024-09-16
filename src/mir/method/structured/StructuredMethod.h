/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 *
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#pragma once

#include <vector>

#include "mir/method/MethodWeighted.h"


namespace mir {
namespace param {
class MIRParametrisation;
}
namespace repres {
class Representation;
}
}  // namespace mir


namespace mir::method::structured {


class StructuredMethod : public MethodWeighted {
public:
    explicit StructuredMethod(const param::MIRParametrisation&);
    ~StructuredMethod() override;

protected:
    // Find nearest West-East bounding i indices
    void left_right_lon_indexes(const Longitude& in, const std::vector<PointLatLon>& coords, size_t start, size_t end,
                                size_t& left, size_t& right) const;


    // Normalize weights triplets such that sum(weights) = 1
    void normalise(std::vector<WeightMatrix::Triplet>& triplets) const;

    // Get lat/lon point coordinates from representation
    void getRepresentationPoints(const repres::Representation&, std::vector<PointLatLon>&, Latitude& minimum,
                                 Latitude& maximum) const;

    // Get latitudes list from representation
    void getRepresentationLatitudes(const repres::Representation&, std::vector<Latitude>&) const;

    // Find nearest North-South bounding j indices
    void boundNorthSouth(const Latitude&, const std::vector<Latitude>&, size_t& jNorth, size_t& jSouth) const;

    // Find nearest West-East bounding i indices
    void boundWestEast(const Longitude&, const size_t& Ni, const size_t& iStart, size_t& iWest, size_t& iEast) const;

    bool sameAs(const Method&) const override = 0;

private:
    void assemble(util::MIRStatistics&, WeightMatrix&, const repres::Representation& in,
                  const repres::Representation& out) const override;

    virtual void assembleStructuredInput(WeightMatrix&, const repres::Representation& in,
                                         const repres::Representation& out) const = 0;

    void json(eckit::JSON&) const override;
};


}  // namespace mir::method::structured
