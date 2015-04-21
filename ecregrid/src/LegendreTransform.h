/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef LegendreTransform_H
#define LegendreTransform_H


// Headers
#ifndef   machine_H
#include "machine.h"
#endif

#include "ref_counted_ptr.h"

// Forward declarations
class LegendrePolynomials;


class LegendreTransform {
  public:

// -- Exceptions
    // None

// -- Contructors

    LegendreTransform(const ref_counted_ptr<const LegendrePolynomials>& poly, int truncation, const vector<double>& lats);

// -- Destructor

    virtual ~LegendreTransform(); // Change to virtual if base class

// -- Convertors
    // None

// -- Operators
    // None

// -- Methods
//	virtual int transform(mapLats& mapped, const vector<comp>& dataComplex, int northIndex, int southIndex) const = 0 ;
//	virtual int transform(double* coeff, const double* data, int latNumber) const = 0;

// -- Overridden methods
    // None

// -- Class members
    // None

// -- Class methods
    // None

  protected:

// -- Members
    int    truncation_;
    ref_counted_ptr<const LegendrePolynomials> polynomials_;
    vector<double> globalLatitudes_;

// -- Methods

    // void print(ostream&) const; // Change to virtual if base class

// -- Overridden methods
    // None

// -- Class members

// -- Class methods
    // None

  private:

// No copy allowed

    LegendreTransform(const LegendreTransform&);
    LegendreTransform& operator=(const LegendreTransform&);

// -- Members

// -- Methods

// -- Overridden methods
    // None

// -- Class members
    // None

// -- Class methods
    // None

// -- Friends

    //friend ostream& operator<<(ostream& s,const LegendreTransform& p)
    //	{ p.print(s); return s; }

};

#endif
