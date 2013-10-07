/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "DoubleLinearLsm.h"

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

DoubleLinearLsm::DoubleLinearLsm(const Grid& input, const Grid& output, const string& lsmMethod) :
	InterpolatorLsm(4,input,output,lsmMethod)
{
}

DoubleLinearLsm::DoubleLinearLsm(bool w, bool a, double nPole, double sPole, const Grid& input, const Grid& output, const string& lsmMethod) :
	InterpolatorLsm(w,a,nPole,sPole,4,input,output,lsmMethod)
{
}

DoubleLinearLsm::~DoubleLinearLsm()
{
}

inline double adjust(double a, double b) { return 1.0 - int(b + 1.e-5) * (1.0 - a) ; }

inline double f2l(double alpha, double beta, double gamma, double delta, double epsilon) 
{ 	return 
	alpha * ((1.0 + beta*gamma)* 
	(1.0 + delta*(gamma-2.0)) * 
	(1.0 + epsilon*(gamma-3.0))) / 
	((1.0 - 2.0*delta) * (1.0 - 3.0*epsilon)); }

inline double f3l(double alpha, double beta, double gamma, double delta, double epsilon) 
{ 	return 
	alpha * ((1.0 + beta*gamma) * 
	(1.0 + delta*(gamma - 1.0)) * 
	(1.0 + epsilon*(gamma - 3.0))) / 
	((1.0 + beta) * (1.0 - 2.0*epsilon)); }

inline double f4l(double alpha, double beta, double gamma, double delta, double epsilon) 
{ 	return 
	alpha * ((1.0 + beta*gamma) *
	(1.0 + delta*(gamma - 1.0)) *
	(1.0 + epsilon*(gamma - 2.0))) /
	((1.0 + 2.0*beta) * (1.0 + delta)) ; }


/*static*/ double DoubleLinearLsm::calculateInterpolatedValue(const Point& where, const vector<FieldPoint>& nearests, const vector<double>& inLsmData, const vector<double>& outLsmData)
{

    // to be called directly we need 4 points at least
    ASSERT(nearests.size() >= 4);

    double zwlsi1  = 1.0;
    double zwlsi2  = 1.0;
    double zwlsi3  = 1.0;
    double zwlsi4  = 1.0;

    double zwlsi14 = 0.0;
    double zwlsi15 = 0.0;
    double zwlsi17 = 0.0;

	double targetLat = where.latitude();
	double targetLon = where.longitude();

	bool target = isLand( outLsmData[where.k1dIndex()] );

	if(isLand( inLsmData[nearests[0].k1dIndex()]) != target)
		zwlsi1 = 0;
	if(isLand( inLsmData[nearests[1].k1dIndex()]) != target)
		zwlsi2 = 0;
	if(isLand( inLsmData[nearests[2].k1dIndex()]) != target)
		zwlsi3 = 0;
	if(isLand( inLsmData[nearests[3].k1dIndex()]) != target)
		zwlsi4 = 0;
		
    zwlsi14 = min(zwlsi1  + zwlsi2   ,1.0);
    zwlsi15 = min(zwlsi3  + zwlsi4   ,1.0);
    zwlsi17 = min(zwlsi14 + zwlsi15  ,1.0);

    zwlsi1  = adjust(zwlsi1,zwlsi14);
    zwlsi2  = adjust(zwlsi2,zwlsi14);
    zwlsi3  = adjust(zwlsi3,zwlsi15);
    zwlsi4  = adjust(zwlsi4,zwlsi15);

    zwlsi14 = adjust(zwlsi17,zwlsi14);
    zwlsi15 = adjust(zwlsi17,zwlsi15);


	double lat1 = nearests[0].latitude();
	double lat2 = nearests[2].latitude();

//     pdlat   - Meridian linear weight
	double pdlat = (targetLat - lat1) / (lat2 - lat1);

//     pdlo1   - Zonal linear weight for the latitude of point 0.
			double step = fabs(nearests[1].longitude() - nearests[0].longitude());
	double pdlo1 = (targetLon - nearests[0].iIndex() * step ) / step;

//     pdlo2   - Zonal linear weight for the latitude of point 2.
			step = fabs(nearests[3].longitude() - nearests[2].longitude());
	double pdlo2 = (targetLon - nearests[2].iIndex() * step ) / step;

    double zwy  = zwlsi15 * (1.0 + zwlsi14 * (pdlat - 1.0));
    double zcy  = 1.0 - zwy;

    double zwxn = zwlsi2 * (1.0 + zwlsi1 * (pdlo1 - 1.0));
    double zcxn = 1.0 - zwxn;

    double zwxs = zwlsi4 * (1.0 + zwlsi3 * (pdlo2 - 1.0));
    double zcxs = 1.0 - zwxs;

//	cout << "DoubleLinearLsm::interpolatedValue  zwxn1: " << zwxn1 << " zwxn2 " <<  zwxn2 << "  pdlo1 " << pdlo1 << endl;
//	cout << " ----------- " << endl;

	return      nearests[0].value() * zcxn * zcy +
				nearests[1].value() * zwxn * zcy +
				nearests[2].value() * zcxs * zwy +
				nearests[3].value() * zwxs * zwy;
}

double DoubleLinearLsm::interpolatedValue(const Point& where, const vector<FieldPoint>& nearests) const
{
	if(nearests.size() < 4)
		return missingNeighbours(where,nearests,nearests.size());

    // NB we can pass dereferenced lsm data ojects here as we hold ref_counted_ptr for
    // them as class members. hence they are guaranteed to exist while this
    // call completes
    return DoubleLinearLsm::calculateInterpolatedValue(where, nearests, *inLsmData_, *outLsmData_);
}

void DoubleLinearLsm::print(ostream& out) const
{
	Interpolator::print(out);
	out << "DoubleLinearLsm" ;
}

