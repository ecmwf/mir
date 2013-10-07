/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef Extraction_H
#define Extraction_H


// Headers
#ifndef   machine_H
#include "machine.h"
#endif

// Forward declarations
class GridField;
class Grid;

class Extraction {
public:

// -- Exceptions
	// None

// -- Contructors

	Extraction();

// -- Destructor

	virtual ~Extraction(); // Change to virtual if base class

// -- Methods
	virtual void extract(const Grid& output, vector<double>& values) const = 0;

protected:
	virtual void print(ostream&) const; // Change to virtual if base class	

private:

// No copy allowed

	Extraction(const Extraction&);
	Extraction& operator=(const Extraction&);

// -- Friends

	friend ostream& operator<<(ostream& s,const Extraction& p)
		{ p.print(s); return s; }

};

#endif
