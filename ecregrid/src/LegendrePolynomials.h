/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef LegendrePolynomials_H
#define LegendrePolynomials_H

// Headers
#ifndef   machine_H
#include "machine.h"
#endif


class LegendrePolynomials {
public:

// -- Exceptions
	// None

// -- Contructors

	LegendrePolynomials(int truncation);

// -- Destructor

	virtual ~LegendrePolynomials(); // Change to virtual if base class


// -- Methods

	void calculateLegendrePoly(double* leg, double* work, double lat) const;

	virtual const double* getOneLatitude(double lat, int rowOffset) const  = 0;
	virtual const double* getPolynoms() const = 0;

protected:
	
// -- Members
	int    truncation_;

// -- Methods

    virtual void print(ostream&) const; // Change to virtual if base class

private:

// No copy allowed

	LegendrePolynomials(const LegendrePolynomials&);
	LegendrePolynomials& operator=(const LegendrePolynomials&);



// -- Friends

	friend ostream& operator<<(ostream& s,const LegendrePolynomials& p)
	{ p.print(s); return s; }

};

#endif
