/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "AverageWeightedRegularLsm.h"

#ifndef Exception_H
#include "Exception.h"
#endif

#ifndef FieldPoint_H
#include "FieldPoint.h"
#endif

#ifndef Grid_H
#include "Grid.h"
#endif

#ifndef GridField_H
#include "GridField.h"
#endif

#ifndef Timer_H
#include <eckit/utils/Timer.h>
#endif

#ifndef GridContext_H
#include "GridContext.h"
#endif

#ifndef DerivedSubgridParameters_H
#include "DerivedSubgridParameters.h"
#endif

#include <algorithm>

AverageWeightedRegularLsm::AverageWeightedRegularLsm(const Grid& in, const Grid& out, const string& lsmMethod) :
	InterpolatorLsm(in,out,lsmMethod), northSouthNumberOfPoints_(out.northSouthNumberOfPoints()), westEastNumberOfPoints_(out.westEastNumberOfPoints()), northSouthIncrement_(out.northSouthIncrement()), westEastIncrement_(out.westEastIncrement())
{
	weights_.reserve(in.calculatedNumberOfPoints());
	in.aWeights(weights_);
	out.latitudes(outLats_);
}

AverageWeightedRegularLsm::~AverageWeightedRegularLsm()
{ 
//cout << "AverageWeightedRegularLsm: cleaning up." << endl; 
}



void AverageWeightedRegularLsm::interpolate(const Grid& input, const vector<double>& data, int inScMode, double missingValue, const vector<Point>& outputPoints, vector<double>& values) const
{
	auto_ptr<GridContext> ctx(input.getGridContext());

	if(DEBUG)
		cout << " AverageWeightedRegularLsm::interpolate START ---> " << endl;

	unsigned long count = 0;
   	for (int j = 0 ; j < northSouthNumberOfPoints_ ; j++) {
   		for (int i = 0 ; i < westEastNumberOfPoints_ ; i++) {
            // NB we can pass dereferenced lsm data ojects here as we hold ref_counted_ptr for
            // them as class members. hence they are guaranteed to exist while this
            // call completes
			values[count] = input.averageWeightedLsm(ctx.get(),outputPoints[count],weights_,data,*inLsmData_,*outLsmData_,inScMode,missingValue,outLats_,westEastIncrement_);
			count++;
		}
	}

	if(DEBUG)
		cout << " AverageWeightedRegularLsm::interpolate END <--- " << endl;

	ASSERT(values.size() == count);
}

void AverageWeightedRegularLsm::standardDeviation(const Grid& input, const vector<double>& data, const vector<double>& dataSquared, int inScMode, double missingValue, const vector<Point>& outputPoints, vector<double>& values) const
{
	auto_ptr<GridContext> ctx(input.getGridContext());

	if(DEBUG)
		cout << " AverageWeightedRegularLsm::standardDeviation START ---> " << endl;

    // NB we can pass dereferenced lsm data ojects here as we hold ref_counted_ptr for
    // them as class members. hence they are guaranteed to exist while this
    // call completes
   	for (unsigned int i = 0 ; i < values.size() ; i++) {
		double Linter = input.averageWeightedLsm(ctx.get(),outputPoints[i],weights_,data,*inLsmData_,*outLsmData_,inScMode,missingValue,outLats_,westEastIncrement_); 
		double Kinter = input.averageWeightedLsm(ctx.get(),outputPoints[i],weights_,dataSquared,*inLsmData_,*outLsmData_,inScMode,missingValue,outLats_,westEastIncrement_); 

		if(same(Linter,missingValue) || same(Kinter,missingValue))
			values[i] = missingValue;
		else
			values[i] = sqrt(max(0.0,(Kinter - Linter*Linter))) ; 
	}

	if(DEBUG)
		cout << " AverageWeightedRegularLsm::standardDeviation END <--- " << endl;
}

void AverageWeightedRegularLsm::derivedSubgridParameters(const Grid& input, const vector<double>& K, const vector<double>& L, const vector<double>& M, int inScMode, double missingValue, const vector<Point>& outputPoints, vector<double>& values, const DerivedSubgridParameters& stat) const
{
    vector<FieldPoint> nearests;
    nearests.reserve(neighbour_);

    auto_ptr<GridContext> ctx(input.getGridContext());

	if(DEBUG)
		cout << " AverageWeightedRegularLsm::derivedSubgridParameters START ---> " << stat << endl;

    // NB we can pass dereferenced lsm data ojects here as we hold ref_counted_ptr for
    // them as class members. hence they are guaranteed to exist while this
    // call completes
   	for (unsigned int i = 0 ; i < values.size() ; i++){
		double Kinter = input.averageWeightedLsm(ctx.get(),outputPoints[i],weights_,K,*inLsmData_,*outLsmData_,inScMode,missingValue,outLats_,westEastIncrement_); 
		double Linter = input.averageWeightedLsm(ctx.get(),outputPoints[i],weights_,L,*inLsmData_,*outLsmData_,inScMode,missingValue,outLats_,westEastIncrement_); 
		double Minter = input.averageWeightedLsm(ctx.get(),outputPoints[i],weights_,M,*inLsmData_,*outLsmData_,inScMode,missingValue,outLats_,westEastIncrement_); 
		if(same(Linter,missingValue) || same(Kinter,missingValue) || same(Minter,missingValue))
			values[i] = missingValue;
		else
			values[i] = stat.calculate(Kinter,Linter,Minter); 
	}

	if(DEBUG)
		cout << " AverageWeightedRegularLsm::derivedSubgridParameters END <--- " << endl;
}

double AverageWeightedRegularLsm::interpolatedValue(const Point& where, const vector<FieldPoint>& nearests) const
{
	throw NotImplementedFeature("AverageWeightedRegularLsm::interpolatedValue");
	return 0;
}

void AverageWeightedRegularLsm::print(ostream& out) const
{
	Interpolator::print(out);
	out << "AverageWeightedRegularLsm";
}
