/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef BinaryOutput_H
#define BinaryOutput_H

#ifndef Output_H
#include "Output.h"
#endif

// Forward declarations
class GridField;

class BinaryOutput : public Output {
public:

// -- Contructors

	BinaryOutput();
	BinaryOutput(const string& name);

// -- Destructor

	~BinaryOutput();

// --  Methods
	virtual void write(FILE* out, const vector<double>& values) const;
	void write(const char* buffer,long length) const;


// Overriden methods
	void write(const Field& field) const;
	void write(FILE* out, const Field& field) const;
	string typeOf()  const { return "binary"; }

private:

// -- Members


// -- Friends

};

#endif
