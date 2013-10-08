/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "LinearFit3TimesLsm.h"

#ifndef Exception_H
#include "Exception.h"
#endif

#ifndef FieldPoint_H
#include "FieldPoint.h"
#endif

#ifndef Grid_H
#include "Grid.h"
#endif

#include <algorithm>

LinearFit3TimesLsm::LinearFit3TimesLsm(const Grid& input, const Grid& output, const string& lsmMethod) :
	InterpolatorLsm(4,input,output,lsmMethod)
{
}

LinearFit3TimesLsm::LinearFit3TimesLsm(bool w, bool a, double nPole, double sPole, const Grid& input, const Grid& output, const string& lsmMethod) :
	InterpolatorLsm(w,a,nPole,sPole,4,input,output,lsmMethod)
{
}

LinearFit3TimesLsm::~LinearFit3TimesLsm()
{ 
//cout << "LinearFit3TimesLsm: cleaning up." << endl; 
}

/*
	Numbering of the points (I is the interpolation point):

                0       1
				    I
                2       3
*/

double LinearFit3TimesLsm::interpolatedValue(const Point& where, const vector<FieldPoint>& nearests) const
{
	int size = nearests.size();
	if(size < 4)
		return missingNeighbours(where,nearests,size);

	double whereLon = where.longitude();
	double whereLat = where.latitude();

	double nwLon = nearests[0].longitude();	
	double nwLat = nearests[0].latitude();	

	double neLon = nearests[1].longitude();	

	double swLon = nearests[2].longitude();	
	double swLat = nearests[2].latitude();	

	double seLon = nearests[3].longitude();	

//	double distNwLon = fabs(nwLon - whereLon);
	double distNwLon = fabs(whereLon - nwLon);
	double distNeLon = fabs(neLon - whereLon);

//	double distSwLon = fabs(swLon - whereLon);
	double distSwLon = fabs(whereLon - swLon);
	double distSeLon = fabs(seLon - whereLon);

	double distNwLat = fabs(nwLat - whereLat);
	double distSwLat = fabs(swLat - whereLat);

	double znorth =  distNwLon + distNeLon;
	double zsouth =  distSwLon + distSeLon;

    double pwfactNW = distNeLon * zsouth * distSwLat;
    double pwfactNE = distNwLon * zsouth * distSwLat;

    double pwfactSW = distSeLon * znorth * distNwLat;
    double pwfactSE = distSwLon * znorth * distNwLat;
/*
	if( where.k1dIndex() == 13962){
			cout << endl << "out " << where.k1dIndex() << " " << isLand( outLsmData[where.k1dIndex()] ) << " lsm " <<  outLsmData[where.k1dIndex()] << endl;
			cout << "1 " << nearests[0].k1dIndex() << " " << isLand( inLsmData[nearests[0].k1dIndex()]) << " lsm " << inLsmData[nearests[0].k1dIndex()] << endl;
			cout << "2 " << nearests[1].k1dIndex() << " " << isLand( inLsmData[nearests[1].k1dIndex()]) << " lsm " << inLsmData[nearests[1].k1dIndex()] << endl;
			cout << "3 " << nearests[2].k1dIndex() << " " << isLand( inLsmData[nearests[2].k1dIndex()]) << " lsm " << inLsmData[nearests[2].k1dIndex()] << endl;
			cout << "4 " << nearests[3].k1dIndex() << " " << isLand( inLsmData[nearests[3].k1dIndex()]) << " lsm " << inLsmData[nearests[3].k1dIndex()] << endl;
	}
	*/

    // dereference lsm data to local reference. This is okay as the data is
    // guaranteed to exist as long as the ref counted pointer exists
    const vector<double>& inLsmData = *inLsmData_;
    const vector<double>& outLsmData = *outLsmData_;

	bool target = isLand( outLsmData[where.k1dIndex()] );
	double weight0 = adjustWeight(target, isLand(inLsmData[nearests[0].k1dIndex()] ) ,pwfactNW);
	double weight1 = adjustWeight(target, isLand(inLsmData[nearests[1].k1dIndex()] ),pwfactNE);
	double weight2 = adjustWeight(target, isLand(inLsmData[nearests[2].k1dIndex()] )  ,pwfactSW);
	double weight3 = adjustWeight(target, isLand(inLsmData[nearests[3].k1dIndex()] ),pwfactSE);

	double sum = weight0 + weight1 + weight2 + weight3;

    return nearests[0].value() * (weight0/sum)
         + nearests[1].value() * (weight1/sum)
         + nearests[2].value() * (weight2/sum)
         + nearests[3].value() * (weight3/sum);
}
 

void LinearFit3TimesLsm::print(ostream& out) const
{
	Interpolator::print(out);
	out << "LinearFit3TimesLsm - Lsm" ;
}
