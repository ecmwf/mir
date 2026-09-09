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

    bool angleApproximatelyEqual(const Latitude&, const Latitude&) const;
    bool angleApproximatelyEqual(const Longitude&, const Longitude&) const;

    void correctSouthNorth(Latitude& s, Latitude& n, bool in = true) const;

    std::vector<double> calculateUnrotatedGridBoxLatitudeEdges() const;

    // -- Overridden methods

    void fillMeshGen(util::MeshGeneratorParameters&) const override;
    bool sameAs(const Representation&) const override;
    void validate(const MIRValuesVector&) const override;

    std::string intersectionOnCrop() const override { return "input-intersects-output"; }

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
