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

#include "mir/repres/gauss/Gaussian.h"


namespace mir::repres::gauss::regular {


class Regular : public Gaussian {
public:
    // -- Exceptions
    // None

    // -- Constructors

    Regular(const param::MIRParametrisation&);
    Regular(size_t N, const util::BoundingBox& = util::BoundingBox(), double angularPrecision = 0);

    // -- Destructor

    ~Regular() override = default;

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods
    // None

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

protected:
    // -- Constructors
    // None

    // -- Members

    size_t k_;
    size_t Ni_;
    size_t Nj_;

    // -- Methods

    void correctWestEast(Longitude& w, Longitude& e) const;
    void setNiNj();

    // -- Overridden methods

    void fillGrib(grib_info&) const override;
    void fillJob(api::MIRJob&) const override;
    bool sameAs(const Representation&) const override;
    atlas::Grid atlasGrid() const override;
    void makeName(std::ostream&) const override;
    util::BoundingBox extendBoundingBox(const util::BoundingBox&) const override;

    // from Representation
    bool isPeriodicWestEast() const override;

    // -- Class members
    // None

    // -- Class methods
    // None

private:
    Regular();

    // -- Members
    // None

    // -- Methods

    eckit::Fraction getSmallestIncrement() const;

    // -- Overridden methods

    // from Representation
    size_t frame(MIRValuesVector&, size_t size, double missingValue, bool estimate = false) const override;
    size_t numberOfPoints() const override;
    bool getLongestElementDiagonal(double&) const override;
    void json(eckit::JSON&) const override;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


}  // namespace mir::repres::gauss::regular
