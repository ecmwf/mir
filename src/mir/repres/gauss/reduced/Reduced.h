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


#ifndef Reduced_H
#define Reduced_H

#include "mir/repres/gauss/Gaussian.h"
#include "mir/util/BoundingBox.h"


namespace mir {
namespace repres {
namespace reduced {

class Reduced : public Gaussian {
  public:

    // -- Exceptions
    // None

    // -- Contructors

    Reduced(const param::MIRParametrisation &parametrisation);
    Reduced(size_t N);
    Reduced(size_t N, const util::BoundingBox &);

    // -- Destructor

    virtual ~Reduced(); // Change to virtual if base class

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

  protected:

    // -- Members

    // -- Methods

    virtual bool globalDomain() const;
    virtual atlas::grid::Domain atlasDomain() const;

    // void print(std::ostream &) const; // Change to virtual if base class

    // -- Overridden methods

    virtual void fill(grib_info &) const;
    virtual void fill(api::MIRJob &) const;
    virtual Iterator* rotatedIterator() const; // After rotation
    virtual Iterator* unrotatedIterator() const; // Before rotation


    // -- Class members
    // None

    // -- Class methods
    // None

  private:



    // No copy allowed

    Reduced(const Reduced &);
    Reduced &operator=(const Reduced &);

    // -- Members

    // -- Methods

    virtual const Reduced *cropped(const util::BoundingBox &bbox, const std::vector<long> &) const ;


    virtual const std::vector<long> &pls() const = 0;


    // -- Overridden methods

    virtual const Reduced* cropped(const util::BoundingBox &bbox) const ;
    virtual void validate(const std::vector<double> &values) const;
    virtual size_t frame(std::vector<double> &values, size_t size, double missingValue) const;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    //friend ostream& operator<<(ostream& s,const Reduced& p)
    //  { p.print(s); return s; }

};

}  // namespace
}  // namespace repres
}  // namespace mir
#endif

