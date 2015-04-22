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


#ifndef RegularLL_H
#define RegularLL_H

#include "soyuz/repres/Gridded.h"


namespace mir {
namespace repres {


class RegularLL : public Gridded {
  public:

// -- Exceptions
    // None

// -- Contructors

    RegularLL(const MIRParametrisation&);

// -- Destructor

    virtual ~RegularLL(); // Change to virtual if base class

// -- Convertors
    // None

// -- Operators
    // None

// -- Methods

    size_t ni() const {
        return ni_;
    }
    size_t nj() const {
        return nj_;
    }

// -- Overridden methods
    // None

// -- Class members
    // None

// -- Class methods
    // None

  protected:

// -- Members
    // None

// -- Methods

    void print(std::ostream&) const; // Change to virtual if base class

// -- Overridden methods
    // None

// -- Class members
    // None

// -- Class methods
    // None

  private:

    RegularLL(double north, double west, double south, double east, double north_south_increment, double west_east_increment);

// No copy allowed

    RegularLL(const RegularLL&);
    RegularLL& operator=(const RegularLL&);

// -- Members

    double north_;
    double west_;
    double south_;
    double east_;

    double north_south_increment_;
    double west_east_increment_;

    size_t ni_;
    size_t nj_;

// -- Methods
    // None

    void setNiNj();

// -- Overridden methods

    virtual void fill(grib_info&) const;
    virtual Representation* crop(double north, double west, double south, double east, const std::vector<double>&, std::vector<double>&) const;
    virtual size_t frame(std::vector<double> &values, size_t size, double missingValue) const;

// -- Class members
    // None

// -- Class methods
    // None

// -- Friends

    //friend ostream& operator<<(ostream& s,const RegularLL& p)
    //	{ p.print(s); return s; }

};


}  // namespace repres
}  // namespace mir
#endif

