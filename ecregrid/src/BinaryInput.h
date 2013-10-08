/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef BinaryInput_H
#define BinaryInput_H

// Headers
#ifndef Input_H
#include "Input.h"
#endif


class BinaryInput : public Input {
public:

// -- Contructors

	BinaryInput();
	BinaryInput(const string& fileName);

// -- Destructor

	virtual ~BinaryInput(); // Change to virtual if base class

// -- Operators
	// None


// -- Overridden methods
	bool       next(FILE* fp);

	virtual Input*  newInput(const string& name)  const;


// -- Methods
	void read(char* buffer, size_t length) const;


protected:

// -- Members
	// None

// -- Methods
	
// -- Overridden methods
	// None

// -- Class members
	// None

// -- Class methods
	// None

private:

// No copy allowed

	BinaryInput(const BinaryInput&);
	BinaryInput& operator=(const BinaryInput&);

// -- Methods
	// None

// -- Class members
	bool         triger_;

// -- Overridden methods
	Grid* defineGridForCheck(const string& path) const;

	virtual void getDoubleValues(const string& path, vector<double>& values)  const;
	virtual bool*   getLsmBoolValues(size_t* valuesLength)            const;
	virtual long*   getReducedGridSpecification(size_t* valuesLength) const;
	virtual void    getLatLonValues(vector<Point>& points)            const;
	string typeOf()  const { return "binary"; }
	// None

};

#endif
