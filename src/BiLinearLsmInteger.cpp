/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "BiLinearLsmInteger.h"

#ifndef Exception_H
#include "Exception.h"
#endif

#ifndef FieldPoint_H
#include "FieldPoint.h"
#endif

/*static*/ const double BiLinearLsmInteger::TRIGGER = 0.5;

BiLinearLsmInteger::BiLinearLsmInteger() :
	Interpolator(4)
{
}

BiLinearLsmInteger::BiLinearLsmInteger(bool w, bool a, double nPole, double sPole) :
	Interpolator(w,a,nPole,sPole,4)
{
}

BiLinearLsmInteger::~BiLinearLsmInteger()
{ 
//cout << "BiLinearLsmInteger: cleaning up." << endl; 
}

/*
	Numbering of the points (I is the interpolation point):

                0       1
				    I
                2       3
*/

double BiLinearLsmInteger::interpolatedValue(const Point& where, const vector<FieldPoint>& nearests) const
{
	int size = nearests.size();
	if(size < neighbour_)
		return missingNeighbours(where,nearests,size);

	/* The unnormalised weights are the sizes of the opposing rectangles. */
	double weight0 = unnormalisedWeight(where, nearests[3]);
	double weight1 = unnormalisedWeight(where, nearests[2]);
	double weight2 = unnormalisedWeight(where, nearests[1]);
	double weight3 = unnormalisedWeight(where, nearests[0]);

	double sum = weight0 + weight1 + weight2 + weight3;

	double temp = nearests[0].value() * (weight0/sum) 
	     + nearests[1].value() * (weight1/sum)
		 + nearests[2].value() * (weight2/sum)
		 + nearests[3].value() * (weight3/sum);
	 if(temp < TRIGGER)
	 	return 0;

	 return 1;
}

void BiLinearLsmInteger::print(ostream& out) const
{
	Interpolator::print(out);
	out << "BiLinearLsmInteger" ;
}
