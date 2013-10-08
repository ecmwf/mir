/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef GribApiOutputSpectral_H
#define GribApiOutputSpectral_H

#ifndef GribApiOutput_H
#include "GribApiOutput.h"
#endif

class GribApiOutputSpectral : public GribApiOutput {
public:

// -- Contructors

	GribApiOutputSpectral();
	GribApiOutputSpectral(const string& name);

// -- Destructor

	~GribApiOutputSpectral();

// --  Methods

private:

// No copy allowed
	GribApiOutputSpectral(const GribApiOutputSpectral&);
	GribApiOutputSpectral& operator=(const GribApiOutputSpectral&);

	void setGrib(grib_handle* h, const Field& out, const grib_values* grib_set, int grib_set_count) const;
	grib_handle* setGrib(const Field& out, const grib_values* grib_set, int grib_set_count) const;

// -- Friends

};

#endif
