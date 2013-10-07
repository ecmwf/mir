/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "NearestNeigbourLsm.h"

#ifndef Exception_H
#include "Exception.h"
#endif

#ifndef Point_H
#include "Point.h"
#endif

#ifndef FieldPoint_H
#include "FieldPoint.h"
#endif

#ifndef Grid_H
#include "Grid.h"
#endif

#include <map>
#include <algorithm>
 
NearestNeigbourLsm::NearestNeigbourLsm(const Grid& input, const Grid& output, const string& lsmMethod) :
	InterpolatorLsm(4,input,output,lsmMethod)
{
}

NearestNeigbourLsm::NearestNeigbourLsm(int n, const Grid& input, const Grid& output, const string& lsmMethod) :
	InterpolatorLsm(n,input,output,lsmMethod)
{
}

NearestNeigbourLsm::~NearestNeigbourLsm()
{ 
}

typedef	map<unsigned int,double> mapDistance;	


/*static*/ double NearestNeigbourLsm::calculateInterpolatedValue(const Point& where, const vector<FieldPoint>& nearests, const vector<double>& inLsmData, const vector<double>& outLsmData)
{
    ASSERT(where.k1dIndex() < (int)outLsmData.size());
	const bool lsm = isLand( outLsmData[where.k1dIndex()] );

	mapDistance d;

	bool matchTheSame = false;

	for (int k = 0; k < (int)nearests.size(); k++) {
		// ssp Check if it is neighbour same sort of point as interpolate one
        ASSERT(nearests[k].k1dIndex() < (int)inLsmData.size());
		if(lsm == isLand( inLsmData[nearests[k].k1dIndex()] )){
			d.insert( pair<int,double>(k,where.quickDistance( nearests[k] )) );
			matchTheSame = true;
		}
	}	
	// if none of nearest points has the same land-sea mask 
	if(!matchTheSame){
		for (int k = 0; k < (int)nearests.size(); k++)
			d.insert( pair<int,double>(k,where.quickDistance( nearests[k] )) );
	}

	int index = min_element(d.begin(), d.end(), Interpolator::comparer)->first;

	return nearests[index].value();

}

double NearestNeigbourLsm::interpolatedValue(const Point& where, const vector<FieldPoint>& nearests) const
{
    // NB we can pass dereferenced lsm data ojects here as we hold ref_counted_ptr for
    // them as class members. hence they are guaranteed to exist while this
    // call completes
    return NearestNeigbourLsm::calculateInterpolatedValue(where, nearests, *inLsmData_, *outLsmData_);
}

void NearestNeigbourLsm::print(ostream& out) const
{
	Interpolator::print(out);
	out << "NearestNeigbourLsm" ;
}

