#ifndef AsciiOutput_H
#define AsciiOutput_H
/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "Output.h"

// Forward declarations
class GridField;

class AsciiOutput : public Output {
public:

// -- Contructors

	AsciiOutput();
	AsciiOutput(const string& name);

// -- Destructor

	~AsciiOutput();

// --  Methods


// Overriden methods
	void     write(FILE* out, const vector<double>& values) const;
	void     write(const Field& field) const;
	void     write(FILE* out, const Field& field) const;
	string   typeOf()  const { return "ascii"; }

private:

// -- Members


// -- Friends

};

#endif
