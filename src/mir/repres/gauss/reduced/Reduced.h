// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/repres/gauss/Gaussian.h"


namespace mir::util {
class Rotation;
}  // namespace mir::util


namespace mir::repres::gauss::reduced {


class Reduced : public Gaussian {
public:
    // -- Exceptions
    // None

    // -- Constructors

    Reduced(const param::MIRParametrisation&);
    Reduced(size_t N, const std::vector<long>& pl, const util::BoundingBox& = util::BoundingBox(),
            double angularPrecision = 0);

    // -- Destructor

    ~Reduced() override;

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods

    static std::vector<long> pls(const std::string&);

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

protected:
    // -- Constructors

    Reduced(size_t N, const util::BoundingBox& = util::BoundingBox(), double angularPrecision = 0);

    // -- Members

    size_t k_;
    size_t Nj_;

    // -- Methods

    Iterator* unrotatedIterator() const;
    Iterator* rotatedIterator(const util::Rotation&) const;

    const std::vector<long>& pls() const;

    void setNj(std::vector<long>, const Latitude& s, const Latitude& n);
    void correctWestEast(Longitude& w, Longitude& e) const;

    // -- Overridden methods

    void fillGrib(grib_info&) const override;
    void fillSpec(CustomSpec&) const override;
    bool sameAs(const Representation&) const override;
    util::BoundingBox extendBoundingBox(const util::BoundingBox&) const override;

    // from Representation
    bool isPeriodicWestEast() const override;
    std::vector<util::GridBox> gridBoxes() const override;

    // -- Class members
    // None

    // -- Class methods
    // None

private:
    // -- Members

    std::vector<long> pl_;

    // -- Methods

    eckit::Fraction getSmallestIncrement() const;

    // -- Overridden methods

    // from Representation
    size_t frame(MIRValuesVector& values, size_t size, double missingValue) const override;
    size_t numberOfPoints() const override;
    bool getLongestElementDiagonal(double&) const override;
    std::string factory() const override;
    void json(eckit::JSON&) const override;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


}  // namespace mir::repres::gauss::reduced
