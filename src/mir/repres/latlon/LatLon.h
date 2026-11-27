// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/repres/Gridded.h"
#include "mir/util/Increments.h"


namespace mir::repres::latlon {


class LatLon : public Gridded {
public:
    // -- Exceptions
    // None

    // -- Constructors

    explicit LatLon(const param::MIRParametrisation&);
    LatLon(const util::Increments&, const util::BoundingBox& = {}, const PointLatLon& reference = {0, 0},
           const std::string& order = {});

    // -- Destructor

    ~LatLon() override;

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods

    size_t Ni() const { return ni_; }

    size_t Nj() const { return nj_; }

    static void globaliseBoundingBox(util::BoundingBox&, const util::Increments&,
                                     const PointLatLon& reference = {0, 0});

    static void correctBoundingBox(util::BoundingBox&, size_t& ni, size_t& nj, const util::Increments&,
                                   const PointLatLon& reference = {0, 0});

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

protected:
    // -- Class members

    /// Of bounds of GRIB keys (legacy input, e.g. encoded to the microdegree truncated inwards, MIR-351), a value
    /// within it of a grid point is of the point; gridSpec input bounds are recovered by ecCodes (to remove with legacy
    /// input)
    static constexpr double PRECISION = 2e-6;

    // -- Members

    const util::Increments increments_;
    const PointLatLon reference_;
    size_t ni_;
    size_t nj_;
    std::string order_;

    // -- Methods
    // None

    // -- Overridden methods

    void fillGrib(grib_info&) const override;
    void fillSpec(CustomSpec&) const override;

    void json(eckit::JSON&) const override;
    void print(std::ostream&) const override;

    void makeName(std::ostream&) const override;

    bool sameAs(const Representation&) const override;

    bool isPeriodicWestEast() const override;
    bool includesNorthPole() const override;
    bool includesSouthPole() const override;

    size_t numberOfPoints() const override;
    bool getLongestElementDiagonal(double&) const override;

    Representation* globalise(data::MIRField&) const override;

    // -- Class members

    // None

    // -- Class methods
    // None

private:
    // -- Members
    // None

    // -- Methods
    // None

    // -- Overridden methods

    void fillMeshGen(util::MeshGeneratorParameters&) const override;

    size_t frame(MIRValuesVector& values, size_t size, double missingValue) const override;

    const std::string& order() const override { return order_; }
    bool valuesInDefaultOrder() const override { return true; }


    void validate(const MIRValuesVector&) const override;

    const LatLon* croppedRepresentation(const util::BoundingBox&) const override;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


}  // namespace mir::repres::latlon
