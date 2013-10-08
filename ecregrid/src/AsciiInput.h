/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef AsciiInput_H
#define AsciiInput_H

// Headers
#ifndef Input_H
#include "Input.h"
#endif

#ifndef FieldDescription_H
#include "FieldDescription.h"
#endif

class AsciiInput : public Input {
public:

// -- Contructors

	AsciiInput();
	AsciiInput(const string& name);

// -- Destructor

	~AsciiInput(); // Change to virtual if base class

// -- Operators
	// None


// -- Overridden methods
	bool       next(FILE* fp);

	Input*  newInput(const string& name)  const;

	bool*   getLsmBoolValues(size_t* valuesLength)            const;
	Field* defineField(const FieldDescription& dc) const;

// -- Methods
	void getLatLonAndValues(vector<Point>& points, vector<double>& values)      const;
	void    getLatLonValues(vector<Point>& points)            const;


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

	AsciiInput(const AsciiInput&);
	AsciiInput& operator=(const AsciiInput&);

// -- Members
	FieldDescription  id_;
	bool              triger_;

// -- Methods
	// None

// -- Overridden methods
	Grid* defineGridForCheck(const string& path) const;

	long*   getReducedGridSpecification(size_t* valuesLength) const;
	virtual void getDoubleValues(const string& name, vector<double>& values)  const;
	string       typeOf()  const { return "ascii"; }

};

#endif
