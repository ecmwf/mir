/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef GribApiOutputGrid_H
#define GribApiOutputGrid_H

#ifndef GribApiOutput_H
#include "GribApiOutput.h"
#endif

class GribApiOutputGrid : public GribApiOutput {
public:

// -- Contructors

	GribApiOutputGrid();
	GribApiOutputGrid(const string& name);

// -- Destructor

	~GribApiOutputGrid();

// --  Methods

private:

// No copy allowed
	GribApiOutputGrid(const GribApiOutputGrid&);
	GribApiOutputGrid& operator=(const GribApiOutputGrid&);

// -- Members


};

#endif
