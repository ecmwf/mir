/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef GribApiInput_H
#define GribApiInput_H

// Headers
#ifndef Input_H
#include "Input.h"
#endif

#include <stdio.h>

#include "grib_api.h"
// 

class GribApiInput : public Input {
public:

// -- Contructors

	GribApiInput(const string& name);
	GribApiInput();

// -- Destructor

	~GribApiInput(); // Change to virtual if base class

// -- Operators
	// None

// -- Methods

	// unlike the other "defineField" methods, which take a set of values
    // this one assumes the value vector is merely allocated, but not filled. 
    // This method fills them from the grib handle.
    Field* defineField(grib_handle* handle, vector<double>& values)  const;
	int    scanningMode(long iScansNegatively, long jScansPositively) const;
	size_t   getReducedGridSpecification(vector<long>& values) const;

// -- Overridden methods
	grib_handle*  next(FILE* fp);

	Input*  newInput( const string& name)  const;



protected:

// -- Members
	// None

// -- Methods
	
// -- Class members
	// None

// -- Class methods
	// None

private:

// No copy allowed

	GribApiInput(const GribApiInput&);
	GribApiInput& operator=(const GribApiInput&);

// -- Members

// -- Methods
//	bool next(FILE* fp)   = 0;
	// None

// -- Overridden methods
	Grid* defineGridForCheck(const string& path) const;

	//double* getDoubleValues(size_t* valuesLength)             const;
	//virtual double* getDoubleValues(const string& path, size_t* valuesLength)  const;
	virtual void getDoubleValues(const string& name, vector<double>& values)  const;
	//double* getDoubleValues(const string& path, size_t  valuesLength)  const;
	virtual bool*   getLsmBoolValues(size_t* valuesLength)            const;
//	long*   getReducedGridSpecification(size_t* valuesLength) const;
	virtual void    getLatLonValues(vector<Point>& points)            const;
	string       typeOf()  const { return "grib"; }


};

#endif
