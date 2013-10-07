/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef LegendrePolynomialsMemoryMap_H
#define LegendrePolynomialsMemoryMap_H


#ifndef LegendrePolynomialsRead_H
#include "LegendrePolynomialsRead.h"
#endif


class LegendrePolynomialsMemoryMap : public LegendrePolynomialsRead {
public:

// -- Contructors

	LegendrePolynomialsMemoryMap(int truncation, const Grid& grid);

// -- Destructor

	~LegendrePolynomialsMemoryMap(); // Change to virtual if base class

protected:

// Overriden methods
	void print(ostream&) const;

private:

// No copy allowed

	LegendrePolynomialsMemoryMap(const LegendrePolynomialsMemoryMap&);
	LegendrePolynomialsMemoryMap& operator=(const LegendrePolynomialsMemoryMap&);

// -- Members
	size_t  length_;
	void*   addr_;
	double* next_;

// -- Overridden methods
	const double* getOneLatitude(double lat, int rowOffset) const;
	const double* getPolynoms() const { return next_; }


// -- Friends
	friend ostream& operator<<(ostream& s,const LegendrePolynomialsMemoryMap& p)
		{ p.print(s); return s; }

};

#endif
