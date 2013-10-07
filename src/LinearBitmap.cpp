/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "LinearBitmap.h"

#ifndef Exception_H
#include "Exception.h"
#endif

#ifndef Point_H
#include "Point.h"
#endif

#ifndef FieldPoint_H
#include "FieldPoint.h"
#endif

LinearBitmap::LinearBitmap(double missingValue) :
	Linear(), missingValue_(missingValue)
{
}

LinearBitmap::LinearBitmap(bool w, bool a, double nPole, double sPole, double missingValue) :
	Linear(w,a,nPole,sPole), missingValue_(missingValue)
{
}

LinearBitmap::~LinearBitmap()
{ 
}
 
double LinearBitmap::interpolatedValue(const Point& where, const vector<FieldPoint>& nearests) const
{
	int size = nearests.size();

	if(size < 2){
		if(size == 0)
			return missingValue_;
		return nearests[0].value();
	}
	vector<FieldPoint>::const_iterator it = nearests.begin(), end = nearests.end();
	for( ; it != end; it++){
		if(same(it->value(),missingValue_))
			return useNearestNeigbour(where,nearests,size,missingValue_);
	}


	return Linear::interpolatedValue(where,nearests);
}

void LinearBitmap::print(ostream& out) const
{
	Interpolator::print(out);
	out << "LinearBitmap" ;
}

