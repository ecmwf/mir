/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "Interpolator.h"

#ifndef Exception_H
#include "Exception.h"
#endif

#ifndef GridField_H
#include "GridField.h"
#endif

#ifndef Grid_H
#include "Grid.h"
#endif


#ifndef NearestNeigbour_H
#include "NearestNeigbour.h"
#endif


#ifndef Linear_H
#include "Linear.h"
#endif

#ifndef LinearBitmap_H
#include "LinearBitmap.h"
#endif

#ifndef Lsm_H
#include "Lsm.h"
#endif

#ifndef Extraction_H
#include "Extraction.h"
#endif

#ifndef Factory_H
#include "Factory.h"
#endif


#ifndef GridContext_H
#include "GridContext.h"
#endif

#ifndef Timer_H
#include <eckit/utils/Timer.h>
#endif

#ifndef PartialDerivatives_H
#include "PartialDerivatives.h"
#endif

#ifndef DerivedSubgridParameters_H
#include "DerivedSubgridParameters.h"
#endif


Interpolator::Interpolator() :
	neighbour_(0), extrapolateLinearOnPole_(false), extrapolateAverageOnPole_(false), averageForNorthPole_(0), averageForSouthPole_(0)
{
}

Interpolator::Interpolator(int n) :
	neighbour_(n), extrapolateLinearOnPole_(false), extrapolateAverageOnPole_(false), averageForNorthPole_(0), averageForSouthPole_(0)
{
}

Interpolator::Interpolator(bool w, bool a, double nPole, double sPole, int n) :
	neighbour_(n), extrapolateLinearOnPole_(w), extrapolateAverageOnPole_(a), averageForNorthPole_(nPole), averageForSouthPole_(sPole)
{
}

Interpolator::~Interpolator()
{
}

void Interpolator::interpolationWeights(const Point& where, const vector<FieldPoint>& nearests, vector<double>& weights) const
{
    // should always be derived
    throw NotImplementedFeature("Interpolator::interpolationWeights");
}

long Interpolator::neighboursNeeded(double nsIn, double nsOut) const
{
		cout << "Interpolator::neighboursNeeded in increment " << nsIn << " out increment " << nsOut << endl;
	if(nsOut <= nsIn)
//		return 4;
		return 16;

	int ratio = int(nsOut / nsIn);

	if(ratio > 3)
		return 64;
	else if (ratio > 2)
		return 36;

	return 36;
}

long Interpolator::ratio(double nsIn, double nsOut) const
{
	if(DEBUG)
		cout << "Interpolator::ratio in increment " << nsIn << " out increment " << nsOut << endl;
	if(nsOut <= nsIn)
		return 1;

	if(DEBUG)
		cout << "Interpolator::ratio " << int(nsOut / nsIn) << endl;

	return int(nsOut / nsIn);
/*
	int ratio = int(nsOut / nsIn);
	if(ratio > 3)
		return 6;
	else if (ratio > 2)
		return 3;

	return 3;
*/
}

void Interpolator::missingNeighbourWeights(const Point& where, const vector<FieldPoint>& nearests, vector<double>& weights) const
{
    if (nearests.size() == 0)
        return;

    weights.resize(nearests.size(), 0.0);
    // TODO interpolate on pole needs handling
    //
    if (nearests.size() <= neighbour_)
    {
        NearestNeigbour nn;
        nn.interpolationWeights(where, nearests, weights);
        stringstream ss;
        for (unsigned int i = 0; i < weights.size(); i++)
            ss << " " << weights[i] << endl;
        cout << "Interpolator::missingNeighbourWeights weights " << ss.str() << endl;
        return;
    }

	throw BadParameter("Interpolator::missingNeighbourWeights");

}

double Interpolator::missingNeighbours(const Point& where, const vector<FieldPoint>& nearests, int size, double missingValue) const
{
	if(size == 0)
		return missingValue;

/* This is used for wind parameters for case Gaussian to Latlon.
   A Gaussian field does not have a line of latitude at the pole,
   so the intention is to provide U and V values at the pole
   which have a "directional" value.
*/
	if(extrapolateLinearOnPole_){
		if(size == 2){
			double lat = where.latitude();
			if((fabs(lat - NORTH_POLE) < AREA_FACTOR) || (fabs(lat - SOUTH_POLE) < AREA_FACTOR)){
				LinearBitmap linear(missingValue);
				return linear.interpolatedValue(where,nearests);
			}
		}
	}
	else if(extrapolateAverageOnPole_){
		double lat = where.latitude();
		if(fabs(lat - NORTH_POLE) < AREA_FACTOR) {
            cout << "missingNeighbours: returning averageForNorthPole_" << endl;
			return averageForNorthPole_;
		}
		if (fabs(lat - SOUTH_POLE) < AREA_FACTOR) {
            cout << "missingNeighbours: returning averageForSouthPole_" << endl;
			return averageForSouthPole_;
		}
	}

	if(size <= neighbour_) {
        cout << "missingNeighbours: returning useNearestNeigbour" << endl;
		return useNearestNeigbour(where,nearests,size,missingValue);
	}

	if(DEBUG)
		cout << "Interpolator::missingNeighbours --> nearest number " << size << endl;
	throw BadParameter("Interpolator::missingNeighbours");
}

double Interpolator::missingNeighbours(const Point& where, const vector<FieldPoint>& nearests, int size) const
{
	ASSERT(size > 0);

/* This is used for wind parameters for case Gaussian to Latlon.
   A Gaussian field does not have a line of latitude at the pole,
   so the intention is to provide U and V values at the pole
   which have a "directional" value.
*/
	if(extrapolateLinearOnPole_){
		if(size == 2){
			double lat = where.latitude();
			if((fabs(lat - NORTH_POLE) < AREA_FACTOR) || (fabs(lat - SOUTH_POLE) < AREA_FACTOR)){
				Linear linear;
				return linear.interpolatedValue(where,nearests);
			}
		}
	}
	else if(extrapolateAverageOnPole_){
		double lat = where.latitude();
		if(fabs(lat - NORTH_POLE) < AREA_FACTOR) {
			return averageForNorthPole_;
		}
		if (fabs(lat - SOUTH_POLE) < AREA_FACTOR) {
			return averageForSouthPole_;
		}
	}

	if(size <= neighbour_) {
		return useNearestNeigbour(where,nearests,size);
	}

	if(DEBUG)
		cout << "Interpolator::missingNeighbours --> nearest number " << size << endl;
	throw BadParameter("Interpolator::missingNeighbours");
}

double Interpolator::useNearestNeigbour(const Point& where, const vector<FieldPoint>& nearests, int size) const
{
	ASSERT(size > 0);
	NearestNeigbour n(size);
	return n.interpolatedValue(where,nearests);
}


double Interpolator::useNearestNeigbour(const Point& where, const vector<FieldPoint>& nearests, int size, double missingValue) const
{
	if(size == 0)
		return missingValue;
	NearestNeigbour n(size);
	return n.interpolatedValue(where,nearests);
}

void Interpolator::extrapolateOnPole(const GridField& input)
{
	extrapolateAverageOnPole_ = input.extrapolateAverageOnPole(averageForNorthPole_,averageForSouthPole_); 
	averageForNorthPole_      = input.extrapolateLinearOnPole();
}

void Interpolator::interpolate(const GridField& input, const Grid& output, vector<double>& values) const
{
	if(DEBUG){
		cout << "****************************************************" << endl;
		cout << "*** Interpolator::interpolate - " << *this <<   endl;
		cout << "****************************************************" << endl;
		if(extrapolateAverageOnPole_)
			cout << " Interpolator::interpolate extrapolate Average On Pole  " << extrapolateAverageOnPole_ << endl;
		if(extrapolateLinearOnPole_)
			cout << " Interpolator::interpolate extrapolate Linear On Pole  " << extrapolateLinearOnPole_ << endl;

	}

#if ECREGRID_TIMER
	eckit::Timer timer("Interpolator::interpolate => Generate Output Grid & Interpolate");
#endif

	vector<Point> outputPoints;
	outputPoints.reserve(values.size());

	output.generateGrid1D(outputPoints);
// for rotation to be implemented
//	output.generateGrid1D(outputPoints,input.grid());
	if(DEBUG)
		cout << " Interpolator::interpolate valuesSize: " << values.size() << "  outputPoints.size() " <<  outputPoints.size() << endl;
	ASSERT(values.size() == outputPoints.size());
	
	int inScMode = input.scanningMode();
	double inMissingValue = input.missingValue();
	if(DEBUG)
		cout << " Interpolator::interpolate input Scanning Mode: " << inScMode << " --- Number of points: " << neighbour_ << endl;
    
    const vector<double>& data = input.data();
    ASSERT(data.size() > 0);

//	eckit::Timer time("interpolation loop");
	interpolate(input.grid(),data,inScMode,inMissingValue,outputPoints,values);

	if(getenv("ECREGRID_CHECK_CONSERVING")){
        // CACHING OPPORTUNITY?
		vector<double> weightsIn;
		long inDataLength = input.dataLength();
		weightsIn.reserve(inDataLength);
		input.grid().aWeights(weightsIn);
		cout << "Interpolator::interpolate CHECK CONSERVING GLOBAL BUDGET INPUT: " << checkConservation(data, weightsIn, inMissingValue) << endl;

		vector<double> weightsOut;
		weightsOut.reserve(values.size());
		output.aWeights(weightsOut);
		cout << "Interpolator::interpolate CHECK CONSERVING GLOBAL BUDGET OUTPUT: " << checkConservation(values, weightsOut, inMissingValue) << endl;
	}
}


void Interpolator::interpolate(const Grid& input, const vector<double>& data, int inScMode, double inMissingValue, const vector<Point>& outputPoints, vector<double>& values) const
{
	vector<FieldPoint> nearests;
	nearests.reserve(neighbour_);
	auto_ptr<GridContext> ctx(input.getGridContext());

	bool checkNearest = false;
	if(CHECK_DUMP_NEAREST)
		checkNearest = true;

	if(DEBUG)
		cout << " Interpolator::interpolate START ---> " << endl;
    
    for (unsigned long i = 0 ; i < values.size() ; i++) {
		input.nearestPoints(ctx.get(),outputPoints[i],nearests,data,inScMode,neighbour_);
		values[i] = interpolatedValue(outputPoints[i],nearests);

//		if(i == 13962){
//			cout.precision(20);
//			cout << "Interpolator::interpolate - " << i << " value: " <<   values[i] <<   endl;
//			cout << "Interpolator::interpolate - " << outputPoints[i] << endl;
//			vector<FieldPoint>::const_iterator it = nearests.begin(), end = nearests.end();
//			for( ; it != end; it++)
//				cout << "nearests " << *it << endl;
//		}

		if(checkNearest){
			checkDumpNearest(outputPoints[i],i,nearests);
		}
 }
	if(DEBUG)
		cout << " Interpolator::interpolate END <--- " << endl;

}


double Interpolator::interpolate(const Grid& input, GridContext* ctx, vector<FieldPoint>& nearests, const vector<double>& data, int inScMode, double inMissingValue, const Point& outputPoint) const
{
	input.nearestPoints(ctx,outputPoint,nearests,data,inScMode,neighbour_);
	return interpolatedValue(outputPoint,nearests);
}

void Interpolator::standardDeviation(const GridField& input, const Grid& output, vector<double>& values) const
{
	const vector<double>&  data        = input.data();
	vector<double>  dataSquared;
    input.squaredData(dataSquared);

	int inScMode = input.scanningMode();
	double inMissingValue = input.missingValue();

	if(DEBUG){
		cout << "****************************************************" << endl;
		cout << "*** Interpolator::standardDeviation - " << *this <<   endl;
		cout << "****************************************************" << endl;
		cout << " Interpolator::standardDeviation input Scanning Mode:  " << inScMode << endl;
	}

	vector<Point> outputPoints;
	output.generateGrid1D(outputPoints);

	size_t outSize = outputPoints.size();
	ASSERT(outSize == values.size());
	
	standardDeviation(input.grid(),data,dataSquared,inScMode,inMissingValue,outputPoints,values);

}

void Interpolator::standardDeviation(const Grid& input, const vector<double>& data, const vector<double>& dataSquared, int inScMode, double inMissingValue, const vector<Point>& outputPoints, vector<double>& values) const
{
	vector<FieldPoint> nearests;
	nearests.reserve(neighbour_);
	auto_ptr<GridContext> ctx(input.getGridContext());

	if(DEBUG)
		cout << " Interpolator::standardDeviation START ---> " << endl;

   	for (unsigned long i = 0 ; i < values.size() ; i++) {
		double Linter = interpolate(input,ctx.get(),nearests,data,inScMode,inMissingValue,outputPoints[i]); 
		double Kinter = interpolate(input,ctx.get(),nearests,dataSquared,inScMode,inMissingValue,outputPoints[i]); 

		if(same(Linter,inMissingValue) || same(Kinter,inMissingValue))
			values[i] = inMissingValue;
		else
			values[i] = sqrt(max(0.0,(Kinter - Linter*Linter))) ; 
	}

	if(DEBUG)
		cout << " Interpolator::standardDeviation END <--- " << endl;

}

void Interpolator::derivedSubgridParameters(const GridField& input, const Grid& output, vector<double>& values, const DerivedSubgridParameters& derived) const
{
	const vector<double>&  data = input.data();

    int inScMode = input.scanningMode();
	double inMissingValue = input.missingValue();

	if(DEBUG){
		cout << "****************************************************" << endl;
		cout << "*** Interpolator::DerivedSubgridParameters - " << *this <<   endl;
		cout << "****************************************************" << endl;

		cout << " Interpolator::DerivedSubgridParameters input Scanning Mode:  " << inScMode << endl;
	}

	vector<Point> inputPoints;
	input.grid().generateGrid1D(inputPoints);
	size_t inSize = inputPoints.size();

	PartialDerivatives derivatives;

    // CACHING OPPORTUNITY:
    vector<double> K(inSize);
    vector<double> L(inSize);
    vector<double> M(inSize);

	derivatives.klm(inputPoints,input.grid(),inScMode,data,K,L,M,inMissingValue);

	vector<Point> outputPoints;
	output.generateGrid1D(outputPoints);

	size_t outSize = outputPoints.size();
	ASSERT(outSize == values.size());

	derivedSubgridParameters(input.grid(),K,L,M,inScMode,inMissingValue,outputPoints,values,derived);
	
}

void Interpolator::derivedSubgridParameters(const Grid& input, const vector<double>& K, const vector<double>& L, const vector<double>& M, int inScMode, double inMissingValue, const vector<Point>& outputPoints, vector<double>& values, const DerivedSubgridParameters& derived) const
{
    vector<FieldPoint> nearests;
    nearests.reserve(neighbour_);

    auto_ptr<GridContext> ctx(input.getGridContext());

	if(DEBUG)
		cout << " Interpolator::derivedSubgridParameters START ---> " << derived << endl;

   	for (unsigned long i = 0 ; i < values.size() ; i++){
		double Kinter = interpolate(input,ctx.get(),nearests,K,inScMode,inMissingValue,outputPoints[i]); 
		double Linter = interpolate(input,ctx.get(),nearests,L,inScMode,inMissingValue,outputPoints[i]); 
		double Minter = interpolate(input,ctx.get(),nearests,M,inScMode,inMissingValue,outputPoints[i]); 
		if(same(Linter,inMissingValue) || same(Kinter,inMissingValue) || same(Minter,inMissingValue))
			values[i] = inMissingValue;
		else
			values[i] = derived.calculate(Kinter,Linter,Minter); 
	}

	if(DEBUG)
		cout << " Interpolator::derivedSubgridParameters END <--- " << endl;
}

void Interpolator::checkValues(const vector<double>& values) const
{
	unsigned long count = 0;
	for(unsigned long ii = 0; ii < values.size(); ii++){
		if(values[ii] > 0){
			cout << "Interpolator::checkValues: " << ii << " count: " << count << " values: " << values[ii] << endl; 
		++count;
		}
	}
	cout << "Interpolator::checkValues count: " << count << endl;
}

double Interpolator::checkConservation(const vector<double>& values, const vector<double>& weights, double missingValue) const
{
    const size_t valuesSize = values.size();
//	cout << "Interpolator::checkConservation valuesSize; " << valuesSize << endl;

	double sumw = 0;
	for(unsigned long i = 0; i < valuesSize; i++){
			sumw  += weights[i];
	}
	double sum = 0;
	for(unsigned long i = 0; i < valuesSize; i++){
		if(!same(values[i],missingValue))
			sum  += (values[i] * weights[i]) / sumw;
	}
//	cout << "Interpolator::checkConservation sum: " << sum << endl;

	return sum;
}

void Interpolator::checkDumpNearest(const Point& target, long tIndex, const vector<FieldPoint>& nearests) const
{
//	if(DUMP_NEAREST || (nearests.size() > 3 && ((target.longitude() +  ROUNDING_FACTOR) >  nearests[1].longitude()) && target.longitude() > nearests[0].longitude())){
	if(DUMP_NEAREST || (nearests.size() > 3 && ((same(target.longitude(),nearests[1].longitude()))) && target.longitude() > nearests[0].longitude())){
			cout << "-----------------------------------------" << endl;
			cout << "* " << tIndex + 1 << " " << target << endl;
			for(size_t j = 0; j < nearests.size() ; j++){
				cout << j + 1 << " Out lon: " << target.longitude() << " " << nearests[j] << endl;
			}
	}
}

void Interpolator::print(ostream& out) const
{
	out << "Interpolation method is: " ;
}

/*static*/ bool Interpolator::comparer(const pair<unsigned int,double>& a, const pair<unsigned int,double>& b) 
{
//    
// THIS SHOULD RETURN TRUE IF a IS LESS THAN b
//
/*
    Numbering of the points (I is the interpolation point):

                0       1
                    I
                2       3
*/

    // If one value smaller than other, it's a simple matter of choosing the highest actual value
    if (!same(a.second, b.second))
        //return a.second > b.second;
        return a.second < b.second;

    // Otherwise we make a systematic choice:
    //
    // Return value stating that highest odd numbered value (favour grid points to the right
    // hand side of the original, or below it - see diagram) is closest
    // or the highest indexed even value 
    //
    // NB we assume a != b for this compare function
    //
    // a   b    closest chosen point
    // ---------------
    // 0,  1 --->  1
    // 0,  2 --->  2
    // 0,  3 --->  3
    // 1,  0 --->  1
    // 1,  2 --->  1
    // 1,  3 --->  3
    // 2,  0 --->  2
    // 2,  1 --->  1
    // 2,  3 --->  3
    // 3,  0 --->  3
    // 3,  1 --->  3
    // 3,  2 --->  3

    //
    return (a.first % 2 == b.first % 2 ? /* both even. say highest index is closest: */ a.first > b.first : /* say highest odd index is closest: */ a.first % 2 > b.first % 2);
}

