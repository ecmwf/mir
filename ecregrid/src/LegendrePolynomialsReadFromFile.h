/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef LegendrePolynomialsReadFromFile_H
#define LegendrePolynomialsReadFromFile_H

#ifndef LegendrePolynomialsRead_H
#include "LegendrePolynomialsRead.h"
#endif

// Forward declarations

// 

class LegendrePolynomialsReadFromFile : public LegendrePolynomialsRead {
public:

// -- Exceptions
	// None

// -- Contructors

	LegendrePolynomialsReadFromFile(int truncation, const Grid& grid);

// -- Destructor

	~LegendrePolynomialsReadFromFile(); // Change to virtual if base class

protected:

// Overriden methods
	void print(ostream&) const;

private:

// No copy allowed

	LegendrePolynomialsReadFromFile(const LegendrePolynomialsReadFromFile&);
	LegendrePolynomialsReadFromFile& operator=(const LegendrePolynomialsReadFromFile&);

// -- Members

    // NB To allow other Legendre polynomial classes to hold pointers to
    // thread-safe cached objects, which obviously must not then be modified,
    // we have const access to Legendre data in those classes. For this class
    // (which holds its own non-thread-safe-cached set of data) to use the
    // same methods we must make its data mutable
	mutable vector<double> polynoms_;
    mutable size_t filePositionPreserve_;

// -- Methods
	// None

// -- Overridden methods
	const double* getOneLatitude(double lat, int rowOffset) const;
	const double* getPolynoms() const { return 0; }

// -- Friends
	friend ostream& operator<<(ostream& s,const LegendrePolynomialsReadFromFile& p)
		{ p.print(s); return s; }

};

#endif
