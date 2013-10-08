/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "NearestNeigbourBitmap.h"

#ifndef Exception_H
#include "Exception.h"
#endif

#ifndef Point_H
#include "Point.h"
#endif

#ifndef FieldPoint_H
#include "FieldPoint.h"
#endif

 
NearestNeigbourBitmap::NearestNeigbourBitmap(double missingValue) :
	NearestNeigbour(4,missingValue)
{
}

NearestNeigbourBitmap::NearestNeigbourBitmap(int n, double missingValue) :
	NearestNeigbour(n,missingValue)
{
}


NearestNeigbourBitmap::~NearestNeigbourBitmap()
{ 
//cout << "NearestNeigbourBitmap: cleaning up." << endl; 
}


double NearestNeigbourBitmap::interpolatedValue(const Point& where, const vector<FieldPoint>& nearests) const
{
	int size = nearests.size();
	/* Set Missing Value if all neighbours are missing */
	if(size == 0)
		return missingValue_;
	
	return NearestNeigbour::interpolatedValue(where,nearests);
}

void NearestNeigbourBitmap::print(ostream& out) const
{
	Interpolator::print(out);
	out << "NearestNeigbourBitmap" ;
}

