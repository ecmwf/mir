/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "BiLinearLsm.h"

#ifndef Exception_H
#include "Exception.h"
#endif

#ifndef FieldPoint_H
#include "FieldPoint.h"
#endif

#ifndef Grid_H
#include "Grid.h"
#endif

BiLinearLsm::BiLinearLsm(const Grid& input, const Grid& output, const string& lsmMethod) :
	InterpolatorLsm(4,input,output,lsmMethod)
{
}

BiLinearLsm::BiLinearLsm(bool w, bool a, double nPole, double sPole, const Grid& input, const Grid& output, const string& lsmMethod) :
	InterpolatorLsm(w,a,nPole,sPole,4,input,output,lsmMethod)
{
}

BiLinearLsm::~BiLinearLsm()
{ 
//cout << "BiLinear: cleaning up." << endl; 
}

/*
	Numbering of the points (I is the interpolation point):

                0       1
				    I
                2       3
*/

double BiLinearLsm::interpolatedValue(const Point& where, const vector<FieldPoint>& nearests) const
{
	int size = nearests.size();
	if(size < 4)
		return missingNeighbours(where,nearests,size);

	/* The unnormalised weights are the sizes of the opposing rectangles. */
	double pwfactNW = unnormalisedWeight(where, nearests[3]);
	double pwfactNE = unnormalisedWeight(where, nearests[2]);
	double pwfactSW = unnormalisedWeight(where, nearests[1]);
	double pwfactSE = unnormalisedWeight(where, nearests[0]);

	bool target = isLand( (*outLsmData_)[where.k1dIndex()] );

	/* Adjust weights for surface fields using land-sea mask */
	double weight0 = adjustWeight(target, isLand((*inLsmData_)[nearests[0].k1dIndex()] ) ,pwfactNW);
	double weight1 = adjustWeight(target, isLand((*inLsmData_)[nearests[1].k1dIndex()] ) ,pwfactNE);
	double weight2 = adjustWeight(target, isLand((*inLsmData_)[nearests[2].k1dIndex()] ) ,pwfactSW);
	double weight3 = adjustWeight(target, isLand((*inLsmData_)[nearests[3].k1dIndex()] ) ,pwfactSE);


	double sum = weight0 + weight1 + weight2 + weight3;

	return nearests[0].value() * (weight0/sum) 
	     + nearests[1].value() * (weight1/sum)
		 + nearests[2].value() * (weight2/sum)
		 + nearests[3].value() * (weight3/sum);
}
 

void BiLinearLsm::print(ostream& out) const
{
	InterpolatorLsm::print(out);
	out << "BiLinearLsm" ;
}
