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


#ifndef mir_repres_gauss_regular_Regular_h
#define mir_repres_gauss_regular_Regular_h

#include "mir/repres/gauss/Gaussian.h"


namespace mir {
namespace repres {
namespace gauss {
namespace regular {


class Regular : public Gaussian {
public:
    // -- Exceptions
    // None

    // -- Constructors

    Regular(const param::MIRParametrisation&);
    Regular(size_t N, const util::BoundingBox& = util::BoundingBox(), double angularPrecision = 0);

    // -- Destructor

    virtual ~Regular() override;  // Change to virtual if base class

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

    size_t Ni_;
    size_t Nj_;

    // -- Methods

    void correctWestEast(Longitude& w, Longitude& e) const;
    void setNiNj();

    // -- Overridden methods

    virtual void fill(grib_info&) const override;
    virtual void fill(api::MIRJob&) const override;
    virtual bool sameAs(const Representation&) const override;
    virtual atlas::Grid atlasGrid() const override;
    virtual void makeName(std::ostream&) const override;
    virtual util::BoundingBox extendBoundingBox(const util::BoundingBox&) const override;

    // from Representation
    virtual bool isPeriodicWestEast() const override;

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

    virtual size_t frame(MIRValuesVector&, size_t size, double missingValue, bool estimate = false) const override;
    virtual size_t numberOfPoints() const override;
    virtual bool getLongestElementDiagonal(double&) const override;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    // friend ostream& operator<<(ostream& s,const Regular& p)
    //  { p.print(s); return s; }
};


}  // namespace regular
}  // namespace gauss
}  // namespace repres
}  // namespace mir


#endif
