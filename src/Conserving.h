#ifndef CONSERVING_H
#define CONSERVING_H
/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "Interpolator.h"

class Conserving : public Interpolator {
public:

// -- Contructors
	Conserving();

// -- Destructor
	~Conserving();

protected:
	void print(ostream&) const;

private:

// -- Methods
// -- Overridden methods
	void  interpolate(const GridField& input, const Grid& output, double* values, unsigned long valuesSize) const;

	double interpolatedValue(const Point& point, const vector<FieldPoint>& nearests) const;
};
#endif
