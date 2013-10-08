/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "Conserving.h"

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

#include <algorithm>

Conserving::Conserving() :
	Interpolator(64)
{
}

Conserving::~Conserving()
{ 
//cout << "Conserving: cleaning up." << endl; 
}


double Conserving::interpolatedValue(const Point& where, const vector<FieldPoint>& nearests) const
{
	throw NotImplementedFeature("Conserving::interpolatedValue");
	return 0;
}

void Conserving::interpolate(const GridField& input, const Grid& output, double* values, unsigned long valuesSize) const
{
	Timer time("interpolation loop conserving");
	if(DEBUG){
		cout << "****************************************************" << endl;
		cout << "*** Conserving::interpolate - " << *this <<   endl;
		cout << "****************************************************" << endl;
		cout << " Conserving::interpolate START ---> " << endl;
	}

//	if(DEBUG)
//		Timer timer("Interpolator::interpolate => Generate Output Grid & Interpolate");
	vector<Point> outputPoints;
	outputPoints.reserve(valuesSize);
	output.generateGrid1D(outputPoints);
	ASSERT(valuesSize == outputPoints.size());
	
	int inScMode = input.scanningMode();
	if(DEBUG)
		cout << " Conserving::interpolate input Scanning Mode: " << inScMode << endl;

	vector<double> data;
	input.accumulatedData(data);

	double inMissingValue = input.missingValue();

	vector<FieldPoint> nearests;
	nearests.reserve(neighbour_);
	auto_ptr<GridContext> ctx(input.grid().getGridContext());

//	int nsSize = output.northSouthNumberOfPoints();
	vector<long> gridSpec;
	size_t nsSize = output.getGridDefinition(gridSpec);

// ssp this could be more precise for gaussian grid
	double nsInc = output.northSouthIncrement();

	unsigned long count = 0;
   	for (size_t j = 0 ; j < nsSize ; j++) {
		long numPts = gridSpec[j];
// ssp to be updated for subarea input
		double weInc =  360.0 / numPts;
   		for (int i = 0 ; i < numPts ; i++) {
			values[count] = input.grid().conserving(ctx.get(),outputPoints[count],data,inScMode,inMissingValue,nsInc,weInc);
            count++;
		}
	}

	if(DEBUG)
		cout << " Conserving::interpolate END <--- " << endl;
	ASSERT(valuesSize == count);
}

void Conserving::print(ostream& out) const
{
	Interpolator::print(out);
	out << "Conserving";
}
