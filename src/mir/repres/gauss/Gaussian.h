// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/repres/Gridded.h"
#include "mir/util/BoundingBox.h"


namespace mir::repres {


class Gaussian : public Gridded {
public:
    // -- Exceptions
    // None

    // -- Constructors

    Gaussian(size_t N, const util::BoundingBox& = util::BoundingBox(), double angularPrecision = 0);
    Gaussian(const param::MIRParametrisation&);

    // -- Destructor

    ~Gaussian() override;

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods

    ///@return global Gaussian latitudes
    static const std::vector<double>& latitudes(size_t N);

    ///@return global Gaussian quadrature weights
    static const std::vector<double>& weights(size_t N);

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

protected:
    // -- Members

    size_t N_;
    double angularPrecision_;

    // -- Methods

    ///@return global Gaussian latitudes
    const std::vector<double>& latitudes() const;

    ///@return global Gaussian quadrature weigths
    const std::vector<double>& weights() const;

    /// Angles equal to the angular precision (as encoded), otherwise to PointLonLat::EPS
    bool angleApproximatelyEqual(double, double) const;

    void correctSouthNorth(double& s, double& n, bool in = true) const;

    struct Row {
        double first;
        double last;
        double increment;
        size_t size;
    };

    /// The longitudes of a row of Ni points around within West/East (the first from West), as of the grid's rows
    /// (eckit::geo), a bound within precision of a longitude including it
    static Row row(long Ni, double west, double east, double precision = 0.);

    std::vector<double> calculateUnrotatedGridBoxLatitudeEdges() const;

    // -- Overridden methods

    void fillMeshGen(util::MeshGeneratorParameters&) const override;
    bool sameAs(const Representation&) const override;
    void validate(const MIRValuesVector&) const override;

    std::string intersectionOnCrop() const override { return "input-intersects-output"; }
    bool valuesInDefaultOrder() const override { return true; }

    bool includesNorthPole() const override;
    bool includesSouthPole() const override;

    void json(eckit::JSON&) const override;

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
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


}  // namespace mir::repres
