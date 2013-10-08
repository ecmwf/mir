/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "AverageLsm.h"

#ifndef Exception_H
#include "Exception.h"
#endif

#ifndef FieldPoint_H
#include "FieldPoint.h"
#endif

#ifndef Grid_H
#include "Grid.h"
#endif

AverageLsm::AverageLsm(const Grid& input, const Grid& output, const string& lsmMethod) :
	InterpolatorLsm(16,input,output,lsmMethod)
{
}

AverageLsm::AverageLsm(int n, const Grid& input, const Grid& output, const string& lsmMethod) :
	InterpolatorLsm(n,input,output,lsmMethod)
{
}

AverageLsm::~AverageLsm()
{ 
//cout << "AverageLsm: cleaning up." << endl; 
}

/*
	Numbering of the points (I is the interpolation point):

                0       1
				    I
                2       3
*/

double AverageLsm::interpolatedValue(const Point& where, const vector<FieldPoint>& nearests) const
{
	int size = nearests.size();

    // NB We can dereference the lsm data objects here as there are class-level
    // ref counted pointers to them
	bool lsm = isLand( (*outLsmData_)[where.k1dIndex()] );
	int count = 0;
	double sum = 0;

    for (int k = 0; k < size; k++) {
        // ssp Check if it is neighbour same sort of point as interpolated one
        if(lsm == isLand((*inLsmData_)[nearests[k].k1dIndex()])){
			sum += nearests[k].value();
			++count;
        }
    }
    // if none of nearest points has the same land-sea mask
    if(!count){
    	for (int k = 0; k < size; k++) {
			sum += nearests[k].value();
			++count;
    	}
    }

	return sum / count;
}
 

void AverageLsm::print(ostream& out) const
{
	Interpolator::print(out);
	out << "Average - Lsm" ;
}
