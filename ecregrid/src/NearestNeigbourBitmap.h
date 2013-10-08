#ifndef NEARESTNEIGBOURBITMAP_H
#define NEARESTNEIGBOURBITMAP_H
/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "NearestNeigbour.h"

class NearestNeigbourBitmap : public NearestNeigbour {
public:

	NearestNeigbourBitmap(double missingValue);
	NearestNeigbourBitmap(int n, double missingValue);

	~NearestNeigbourBitmap();

// Overriden methods
	double interpolatedValue(const Point& where, const vector<FieldPoint>& nearests) const;
protected:
	void print(ostream&) const;
private:
	double missingValue_;

};
#endif
