/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Pedro Maciel
/// @date Apr 2015


#ifndef Regular_H
#define Regular_H

#include "mir/repres/gauss/Gaussian.h"
#include "mir/util/BoundingBox.h"


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
    Regular(size_t N);

    // -- Destructor

    virtual ~Regular(); // Change to virtual if base class

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

    Regular(size_t N, const util::BoundingBox&, bool correctBoundingBox);

    // -- Members

    size_t Ni_;

    size_t Nj_;

    // -- Methods

    void setNiNj();
    void checkNiNj() const;

    // -- Overridden methods

    virtual void fill(grib_info&) const;
    virtual void fill(api::MIRJob&) const;
    virtual bool sameAs(const Representation&) const;
    virtual bool getLongestElementDiagonal(double&) const;
    virtual atlas::Grid atlasGrid() const;
    virtual void makeName(std::ostream&) const;

    // -- Class members
    // None

    // -- Class methods
    // None

private:

    Regular();

    // -- Members
    // None

    // -- Methods
    // None

    // -- Overridden methods

    virtual util::BoundingBox croppedBoundingBox(const util::BoundingBox&) const;
    virtual void shape(size_t& ni, size_t& nj) const;
    virtual size_t frame(std::vector<double>& values, size_t size, double missingValue) const;
    size_t numberOfPoints() const;
    eckit::Fraction getSmallestIncrement() const;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    //friend ostream& operator<<(ostream& s,const Regular& p)
    //  { p.print(s); return s; }

};


}  // namespace regular
}  // namespace gauss
}  // namespace repres
}  // namespace mir


#endif

