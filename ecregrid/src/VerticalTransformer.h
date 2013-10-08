#ifndef VerticalTransformer_H
#define VerticalTransformer_H
/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/


// Headers
#include "machine.h"

// Forward declarations
class Field;

// 

class VerticalTransformer {
public:

// -- Contructors

	VerticalTransformer(const string& typeOfLevel, int level);

// -- Destructor

	virtual ~VerticalTransformer(); // Change to virtual if base class

// -- Methods
	virtual Field* transform(const Field& inputUp, const Field& inputDown) const = 0;

private:

	VerticalTransformer(const VerticalTransformer&);
	VerticalTransformer& operator=(const VerticalTransformer&);

	string typeOfLevel_;
	int    level_;
	
};

#endif
