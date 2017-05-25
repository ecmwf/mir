/*
 * (C) Copyright 1996-2015 ECMWF.
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
namespace regular {


class Regular : public Gaussian {
public:

    // -- Exceptions
    // None

    // -- Contructors

    Regular(const param::MIRParametrisation&);
    Regular(size_t N);
    Regular(size_t N, const util::BoundingBox& bbox);

    // -- Destructor

    virtual ~Regular(); // Change to virtual if base class

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods
    // None

    // -- Overridden methods
    virtual void validate(const std::vector<double>&) const;

    // -- Class members
    // None

    // -- Class methods
    // None

protected:

    // -- Members

    size_t Ni_;

    size_t Nj_;

    // -- Methods

    void setNiNj();

    // -- Overridden methods

    virtual Iterator* rotatedIterator() const; // After rotation

    virtual Iterator* unrotatedIterator() const; // Before rotation

    virtual void fill(grib_info&) const;

    virtual void fill(api::MIRJob&) const;

    virtual size_t frame(std::vector<double> &values, size_t size, double missingValue) const;

    virtual atlas::Grid atlasGrid() const;

    virtual util::Domain domain() const;

    virtual util::Domain domain(const util::BoundingBox&) const;

    // -- Class members
    // None

    // -- Class methods
    // None

private:

    Regular();

    // No copy allowed

    Regular(const Regular&);
    Regular& operator=(const Regular&);

    // -- Members
    // None

    // -- Methods
    // None

    // -- Overridden methods

    virtual void shape(size_t &ni, size_t &nj) const;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    //friend ostream& operator<<(ostream& s,const Regular& p)
    //  { p.print(s); return s; }

};


}  // namespace regular
}  // namespace repres
}  // namespace mir


#endif

