/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef Output_H
#define Output_H

#ifndef   machine_H
#include "machine.h"
#endif



// Forward declarations
class Field;
class GridField;
class FieldDescription;
class Input;


class Output {
public:

// -- Exceptions
	// None

// -- Contructors

	Output();
	Output(const string& name);

// -- Destructor

	virtual ~Output(); // Change to virtual if base class

// -- Convertors
	// None

// -- Operators
	// None

// -- Methods

//	Field* defineField(const FieldDescription& dc)          const;

	void deliverData(const Field& field, vector<double>& values) const;
	virtual void      write(FILE* out, const vector<double>& values) const = 0;
	virtual void      write(const Field& field)                       const = 0;
	virtual void      write(FILE* out, const Field& field)            const = 0;
	virtual string    typeOf()  const = 0;


// -- Overridden methods
	// None

// -- Class members
	// None

// -- Class methods
	// None

protected:

// -- Members
	string        fileName_;

// -- Methods
	
	 virtual void print(ostream&) const; // Change to virtual if base class	

// -- Overridden methods
	// None

// -- Class members
	// None

// -- Class methods
	// None

private:

// No copy allowed

	Output(const Output&);
	Output& operator=(const Output&);

// -- Members

// -- Methods
	// None

// -- Overridden methods
	// None

// -- Class members

};

#endif
