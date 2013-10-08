/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef LsmLatLon1km_H
#define LsmLatLon1km_H

#ifndef Lsm_H
#include "Lsm.h"
#endif


// Forward declarations
class Grid;
// 

class LsmLatLon1km : public Lsm {
public:

// -- Exceptions
	// None

// -- Contructors

	LsmLatLon1km();

// -- Destructor

	~LsmLatLon1km(); // Change to virtual if base class

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

// -- Members
	// None

// -- Methods
	
	void print(ostream&) const; // Change to virtual if base class	

// -- Overridden methods
	// None

// -- Class members
	// None

// -- Class methods
	// None

private:

// No copy allowed

	LsmLatLon1km(const LsmLatLon1km&);
	LsmLatLon1km& operator=(const LsmLatLon1km&);

// -- Members
	double* llData_;

// -- Methods
	// None

// -- Overridden methods
	bool*  getLsmValues(const Grid& grid) ;
	// None

// -- Class members
	// None

// -- Class methods
	// None

// -- Friends

	friend ostream& operator<<(ostream& s,const LsmLatLon1km& p)
		{ p.print(s); return s; }

};

#endif
