/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "FluxConservingRegular.h"

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
#include "Timer.h"
#endif

#ifndef GridContext_H
#include "GridContext.h"
#endif

#ifndef DerivedSubgridParameters_H
#include "DerivedSubgridParameters.h"
#endif

#include <algorithm>

FluxConservingRegular::FluxConservingRegular(const Grid& in, const Grid& out) :
	Interpolator(64), northSouthNumberOfPoints_(out.northSouthNumberOfPoints()), westEastNumberOfPoints_(out.westEastNumberOfPoints()), northSouthIncrement_(out.northSouthIncrement()), westEastIncrement_(out.westEastIncrement())
{
	size_t length = in.calculatedNumberOfPoints();

	cellArea_.reserve(length);
	cellAreaSize_.reserve(length);

	in.cellsAreas(cellArea_,cellAreaSize_);
}

FluxConservingRegular::~FluxConservingRegular()
{ 
//cout << "FluxConservingRegular: cleaning up." << endl; 
}



void FluxConservingRegular::interpolate(const Grid& input, const double* data, int inScMode, double missingValue, const vector<Point>& outputPoints, double* values, unsigned long valuesSize) const
{
	auto_ptr<GridContext> ctx(input.getGridContext());
// ssp northSouthIncrement_ could be more precise for gaussian grid

	if(DEBUG)
		cout << " FluxConservingRegular::interpolate START ---> " << endl;

	unsigned long count = 0;
   	for (int j = 0 ; j < northSouthNumberOfPoints_ ; j++) {
   		for (int i = 0 ; i < westEastNumberOfPoints_ ; i++) {
			values[count] = input.fluxConserving(ctx.get(),outputPoints[count],cellArea_,cellAreaSize_,data,inScMode,missingValue,northSouthIncrement_,westEastIncrement_);
			count++;
		}
	}

	if(DEBUG)
		cout << " FluxConservingRegular::interpolate END <--- " << endl;

	ASSERT(valuesSize == count);
}

void FluxConservingRegular::standardDeviation(const Grid& input, const double* data, const double* dataSquared, int inScMode, double missingValue, const vector<Point>& outputPoints, double* values, unsigned long valuesSize) const
{
	auto_ptr<GridContext> ctx(input.getGridContext());

	if(DEBUG)
		cout << " FluxConservingRegular::standardDeviation START ---> " << endl;

   	for (unsigned int i = 0 ; i < valuesSize ; i++) {
		double Linter = input.fluxConserving(ctx.get(),outputPoints[i],cellArea_,cellAreaSize_,data,inScMode,missingValue,northSouthIncrement_,westEastIncrement_); 
		double Kinter = input.fluxConserving(ctx.get(),outputPoints[i],cellArea_,cellAreaSize_,dataSquared,inScMode,missingValue,northSouthIncrement_,westEastIncrement_); 

		if(same(Linter,missingValue) || same(Kinter,missingValue))
			values[i] = missingValue;
		else
			values[i] = sqrt(max(0.0,(Kinter - Linter*Linter))) ; 
	}

	if(DEBUG)
		cout << " FluxConservingRegular::standardDeviation END <--- " << endl;
}

void FluxConservingRegular::derivedSubgridParameters(const Grid& input, const double* K, const double* L, const double* M, int inScMode, double missingValue, const vector<Point>& outputPoints, double* values, unsigned long valuesSize, const DerivedSubgridParameters& derived) const
{
    vector<FieldPoint> nearests;
    nearests.reserve(neighbour_);

    auto_ptr<GridContext> ctx(input.getGridContext());

	if(DEBUG)
		cout << " FluxConservingRegular::derivedSubgridParameters START ---> " << derived << endl;

   	for (unsigned int i = 0 ; i < valuesSize ; i++){
		double Kinter = input.fluxConserving(ctx.get(),outputPoints[i],cellArea_,cellAreaSize_,K,inScMode,missingValue,northSouthIncrement_,westEastIncrement_); 
		double Linter = input.fluxConserving(ctx.get(),outputPoints[i],cellArea_,cellAreaSize_,L,inScMode,missingValue,northSouthIncrement_,westEastIncrement_); 
		double Minter = input.fluxConserving(ctx.get(),outputPoints[i],cellArea_,cellAreaSize_,M,inScMode,missingValue,northSouthIncrement_,westEastIncrement_); 
		if(same(Linter,missingValue) || same(Kinter,missingValue) || same(Minter,missingValue))
			values[i] = missingValue;
		else
			values[i] = derived.calculate(Kinter,Linter,Minter); 
	}

	if(DEBUG)
		cout << " FluxConservingRegular::derivedSubgridParameters END <--- " << endl;
}

double FluxConservingRegular::interpolatedValue(const Point& where, const vector<FieldPoint>& nearests) const
{
	throw NotImplementedFeature("FluxConservingRegular::interpolatedValue");
	return 0;
}

void FluxConservingRegular::print(ostream& out) const
{
	Interpolator::print(out);
	out << "FluxConservingRegular";
}
