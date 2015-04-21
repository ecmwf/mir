/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef LegendrePolynomialsOnFly_H
#define LegendrePolynomialsOnFly_H

#ifndef LegendrePolynomials_H
#include "LegendrePolynomials.h"
#endif

// Forward declarations

//

class LegendrePolynomialsOnFly : public LegendrePolynomials {
  public:

// -- Exceptions
    // None

// -- Contructors
    LegendrePolynomialsOnFly(int truncation);
// -- Destructor
    ~LegendrePolynomialsOnFly(); // Change to virtual if base class


  protected:

// Overriden methods
    void print(ostream&) const;

  private:

    // NB To allow other Legendre polynomial classes to hold pointers to
    // thread-safe cached objects, which obviously must not then be modified,
    // we have const access to Legendre data in those classes. For this class
    // (which holds its own non-thread-safe-cached set of data) to use the
    // same methods we must make its data mutable
    mutable vector<double> polynoms_;
    mutable vector<double> work_;

// No copy allowed
    LegendrePolynomialsOnFly(const LegendrePolynomialsOnFly&);
    LegendrePolynomialsOnFly& operator=(const LegendrePolynomialsOnFly&);

// -- Overridden methods
    const double* getOneLatitude(double lat, int rowOffset) const;
    const double* getPolynoms() const {
        return 0;
    }

// -- Friends
    friend ostream& operator<<(ostream& s,const LegendrePolynomialsOnFly& p) {
        p.print(s);
        return s;
    }
};

#endif
