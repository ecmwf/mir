/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "LinearFit3TimesBitmap.h"

#ifndef Exception_H
#include "Exception.h"
#endif

#ifndef FieldPoint_H
#include "FieldPoint.h"
#endif

LinearFit3TimesBitmap::LinearFit3TimesBitmap(double missingValue) :
	LinearFit3Times(), missingValue_(missingValue)
{
}

LinearFit3TimesBitmap::LinearFit3TimesBitmap(bool w, bool a, double nPole, double sPole, double missingValue) :
	LinearFit3Times(w,a,nPole,sPole), missingValue_(missingValue)
{
}

LinearFit3TimesBitmap::~LinearFit3TimesBitmap()
{ 
//cout << "LinearFit3TimesBitmap: cleaning up." << endl; 
}

 
double LinearFit3TimesBitmap::interpolatedValue(const Point& where, const vector<FieldPoint>& nearests) const
{
	int size = nearests.size();

	if(size < neighbour_)
		return missingNeighbours(where,nearests,size,missingValue_);

	if(same(nearests[0].value(),missingValue_) || same(nearests[1].value(),missingValue_) || same(nearests[2].value(),missingValue_) || same(nearests[3].value(),missingValue_) )
		return useNearestNeigbour(where,nearests,size,missingValue_);

	return LinearFit3TimesBitmap::interpolatedValue(where,nearests);
}

void LinearFit3TimesBitmap::print(ostream& out) const
{
	Interpolator::print(out);
	out << "LinearFit3TimesBitmap" ;
}
