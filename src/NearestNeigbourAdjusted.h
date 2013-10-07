#ifndef NEARESTNEIGBOURADJUSTED_H
#define NEARESTNEIGBOURADJUSTED_H
/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "Interpolator.h"

class NearestNeigbourAdjusted : public Interpolator {
public:

	NearestNeigbourAdjusted();
	NearestNeigbourAdjusted(int n);

	~NearestNeigbourAdjusted();

// Overriden methods
	double interpolatedValue(const Point& where, const vector<FieldPoint>& nearests) const;

protected:
	void print(ostream&) const;

private:

};
#endif
