/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "BiLinearBitmap.h"

#ifndef Exception_H
#include "Exception.h"
#endif

#ifndef FieldPoint_H
#include "FieldPoint.h"
#endif

BiLinearBitmap::BiLinearBitmap(double missingValue) :
	BiLinear(), missingValue_(missingValue)
{
}

BiLinearBitmap::BiLinearBitmap(bool w, bool a, double nPole, double sPole, double missingValue) :
	BiLinear(w,a,nPole,sPole), missingValue_(missingValue)
{
}

BiLinearBitmap::~BiLinearBitmap()
{ 
//cout << "BiLinear: cleaning up." << endl; 
}

/*
	Numbering of the points (I is the interpolation point):

                0       1
				    I
                2       3
*/

double BiLinearBitmap::interpolatedValue(const Point& where, const vector<FieldPoint>& nearests) const
{
	int size = nearests.size();
	if(size < neighbour_)
		return missingNeighbours(where,nearests,size,missingValue_);

	if(same(nearests[0].value(),missingValue_) || same(nearests[1].value(),missingValue_) || same(nearests[2].value(),missingValue_) || same(nearests[3].value(),missingValue_) )
		return useNearestNeigbour(where,nearests,size,missingValue_);

	return BiLinear::interpolatedValue(where,nearests);
}

void BiLinearBitmap::print(ostream& out) const
{
	Interpolator::print(out);
	out << "BiLinearBitmap" ;
}
