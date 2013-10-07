/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "Cubic12ptsBitmap.h"

#ifndef Exception_H
#include "Exception.h"
#endif

#ifndef FieldPoint_H
#include "FieldPoint.h"
#endif


Cubic12ptsBitmap::Cubic12ptsBitmap(double missingValue) :
	Cubic12pts(), missingValue_(missingValue)
{
}

Cubic12ptsBitmap::Cubic12ptsBitmap(bool w, bool a, double nPole, double sPole, double missingValue) :
	Cubic12pts(w,a,nPole,sPole), missingValue_(missingValue)
{
}

Cubic12ptsBitmap::~Cubic12ptsBitmap()
{
}


double Cubic12ptsBitmap::interpolatedValue(const Point& where, const vector<FieldPoint>& nearests) const
{
//	cout << "Cubic12ptsBitmap::interpolatedValue  " << nearests.size() << endl;

	int size = nearests.size();

	if(size < 4){
		return missingNeighbours(where,nearests,size,missingValue_);
	}

	vector<FieldPoint>::const_iterator it = nearests.begin(), end = nearests.end();
	for( ; it != end; it++){
		if(same(it->value(),missingValue_))
			return useNearestNeigbour(where,nearests,size,missingValue_);
	}

	
	return Cubic12pts::interpolatedValue(where,nearests);
}

void Cubic12ptsBitmap::print(ostream& out) const
{
	Interpolator::print(out);
	out << "Cubic12ptsBitmap" ;
}

