/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "AverageWeightedReduced.h"

#include "Exception.h"
#include "FieldPoint.h"
#include "Grid.h"
#include "GridField.h"
#include <eckit/utils/Timer.h>
#include "GridContext.h"
#include "DerivedSubgridParameters.h"

#include <algorithm>

AverageWeightedReduced::AverageWeightedReduced(const Grid& in, const Grid& out) :
	Interpolator(64), northSouthNumberOfPoints_(out.northSouthNumberOfPoints()), northSouthIncrement_(out.northSouthIncrement()), westEastGlobal_(out.isGlobalWestEast()), east_(out.east()), west_(out.west())
{
	sizeSpec_ = out.getGridDefinition(gridSpec_);
	ASSERT(northSouthNumberOfPoints_ == (long)sizeSpec_);

	weights_.reserve(in.calculatedNumberOfPoints());
	in.aWeights(weights_);
	out.latitudes(outLats_);
}

AverageWeightedReduced::~AverageWeightedReduced()
{ 
//cout << "AverageWeightedReduced: cleaning up." << endl; 
}

void AverageWeightedReduced::interpolate(const Grid& input, const vector<double>& data, int inScMode, double missingValue, const vector<Point>& outputPoints, vector<double>& values) const
{
	auto_ptr<GridContext> ctx(input.getGridContext());

	if(DEBUG)
		cout << " AverageWeightedReduced::interpolate START ---> " << endl;

	unsigned long count = 0;
   	for (size_t j = 0 ; j < sizeSpec_ ; j++) {
		long numPts = gridSpec_[j];
		if(!numPts)
			continue;
		double westEastInc =  westEastIncrement(numPts);
//			cout << "reduced numPts: " << numPts << endl;
   		for (int i = 0 ; i < numPts ; i++) {
			values[count] = input.averageWeighted(ctx.get(),outputPoints[count],weights_,data,inScMode,missingValue,outLats_,westEastInc);
			count++;
		}
	}

	if(DEBUG)
		cout << " AverageWeightedReduced::interpolate END <--- " << endl;

	ASSERT(values.size() == count);
}

void AverageWeightedReduced::standardDeviation(const Grid& input, const vector<double>& data, const vector<double>& dataSquared, int inScMode, double missingValue, const vector<Point>& outputPoints, vector<double>& values) const
{
	auto_ptr<GridContext> ctx(input.getGridContext());

	if(DEBUG)
		cout << " AverageWeightedReduced::standardDeviation START ---> " << endl;

	unsigned long count = 0;
   	for (size_t j = 0 ; j < sizeSpec_ ; j++) {
		long numPts = gridSpec_[j];
		if(!numPts)
			continue;
		double westEastInc =  westEastIncrement(numPts);
   		for (int i = 0 ; i < numPts ; i++) {
			double Linter = input.averageWeighted(ctx.get(),outputPoints[count],weights_,data,inScMode,missingValue,outLats_,westEastInc);
			double Kinter = input.averageWeighted(ctx.get(),outputPoints[count],weights_,dataSquared,inScMode,missingValue,outLats_,westEastInc);
			
			if(same(Linter,missingValue) || same(Kinter,missingValue))
				values[count] = missingValue;
			else
				values[count] = sqrt(max(0.0,(Kinter - Linter*Linter))) ; 

				count++;
		}
	}

	if(DEBUG)
		cout << " AverageWeightedReduced::standardDeviation END <--- " << endl;
}

void AverageWeightedReduced::derivedSubgridParameters(const Grid& input, const vector<double>& K, const vector<double>& L, const vector<double>& M, int inScMode, double missingValue, const vector<Point>& outputPoints, vector<double>& values, const DerivedSubgridParameters& stat) const
{
    vector<FieldPoint> nearests;
    nearests.reserve(neighbour_);

    auto_ptr<GridContext> ctx(input.getGridContext());

	if(DEBUG)
		cout << " AverageWeightedReduced::derivedSubgridParameters START ---> " << stat << endl;
	unsigned long count = 0;
   	for (size_t j = 0 ; j < sizeSpec_ ; j++) {
		long numPts = gridSpec_[j];
		if(!numPts)
			continue;
		double westEastInc = westEastIncrement(numPts);
   		for (long i = 0 ; i < numPts ; i++) {
			double Kinter = input.averageWeighted(ctx.get(),outputPoints[count],weights_,K,inScMode,missingValue,outLats_,westEastInc); 
			double Linter = input.averageWeighted(ctx.get(),outputPoints[count],weights_,L,inScMode,missingValue,outLats_,westEastInc); 
			double Minter = input.averageWeighted(ctx.get(),outputPoints[count],weights_,M,inScMode,missingValue,outLats_,westEastInc); 

			if(same(Linter,missingValue) || same(Kinter,missingValue) || same(Minter,missingValue))
				values[count] = missingValue;
			else
				values[count] = stat.calculate(Kinter,Linter,Minter); 

			count++;
		}
	}

	if(DEBUG)
		cout << " AverageWeightedReduced::derivedSubgridParameters END <--- " << endl;
}

double AverageWeightedReduced::interpolatedValue(const Point& where, const vector<FieldPoint>& nearests) const
{
	throw NotImplementedFeature("AverageWeightedReduced::interpolatedValue");
	return 0;
}

void AverageWeightedReduced::print(ostream& out) const
{
	Interpolator::print(out);
	out << "AverageWeightedReduced";
}
