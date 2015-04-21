/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "AverageWeightedRegular.h"

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

AverageWeightedRegular::AverageWeightedRegular(const Grid& in, const Grid& out) :
    Interpolator(64), northSouthNumberOfPoints_(out.northSouthNumberOfPoints()), westEastNumberOfPoints_(out.westEastNumberOfPoints()), northSouthIncrement_(out.northSouthIncrement()), westEastIncrement_(out.westEastIncrement()) {
    weights_.reserve(in.calculatedNumberOfPoints());
    in.aWeights(weights_);

    out.latitudes(outLats_);
}

AverageWeightedRegular::~AverageWeightedRegular() {
//cout << "AverageWeightedRegular: cleaning up." << endl;
}



void AverageWeightedRegular::interpolate(const Grid& input, const vector<double>& data, int inScMode, double missingValue, const vector<Point>& outputPoints, vector<double>& values) const {
    auto_ptr<GridContext> ctx(input.getGridContext());
// ssp northSouthIncrement_ could be more precise for gaussian grid

    if(DEBUG)
        cout << " AverageWeightedRegular::interpolate START ---> " << endl;

    unsigned long count = 0;
    for (int j = 0 ; j < northSouthNumberOfPoints_ ; j++) {
        for (int i = 0 ; i < westEastNumberOfPoints_ ; i++) {
            values[count] = input.averageWeighted(ctx.get(),outputPoints[count],weights_,data,inScMode,missingValue,outLats_,westEastIncrement_);
            count++;
        }
    }

    if(DEBUG)
        cout << " AverageWeightedRegular::interpolate END <--- " << endl;

    ASSERT(values.size() == count);
}



void AverageWeightedRegular::standardDeviation(const Grid& input, const vector<double>& data, const vector<double>& dataSquared,  int inScMode, double missingValue, const vector<Point>& outputPoints, vector<double>& values) const {
    auto_ptr<GridContext> ctx(input.getGridContext());

    if(DEBUG)
        cout << " AverageWeightedRegular::standardDeviation START ---> " << endl;

    for (unsigned int i = 0 ; i < values.size() ; i++) {
        double Linter = input.averageWeighted(ctx.get(),outputPoints[i],weights_,data,inScMode,missingValue,outLats_,westEastIncrement_);
        double Kinter = input.averageWeighted(ctx.get(),outputPoints[i],weights_,dataSquared,inScMode,missingValue,outLats_,westEastIncrement_);

        if(same(Linter,missingValue) || same(Kinter,missingValue))
            values[i] = missingValue;
        else
            values[i] = sqrt(max(0.0,(Kinter - Linter*Linter))) ;
    }

    if(DEBUG)
        cout << " AverageWeightedRegular::standardDeviation END <--- " << endl;
}

void AverageWeightedRegular::derivedSubgridParameters(const Grid& input, const vector<double>& K, const vector<double>& L, const vector<double>& M, int inScMode, double missingValue, const vector<Point>& outputPoints, vector<double>& values, const DerivedSubgridParameters& derived) const {
    vector<FieldPoint> nearests;
    nearests.reserve(neighbour_);

    auto_ptr<GridContext> ctx(input.getGridContext());

    if(DEBUG)
        cout << " AverageWeightedRegular::derivedSubgridParameters START ---> " << derived << endl;

    for (unsigned int i = 0 ; i < values.size() ; i++) {
        double Kinter = input.averageWeighted(ctx.get(),outputPoints[i],weights_,K,inScMode,missingValue,outLats_,westEastIncrement_);
        double Linter = input.averageWeighted(ctx.get(),outputPoints[i],weights_,L,inScMode,missingValue,outLats_,westEastIncrement_);
        double Minter = input.averageWeighted(ctx.get(),outputPoints[i],weights_,M,inScMode,missingValue,outLats_,westEastIncrement_);
        if(same(Linter,missingValue) || same(Kinter,missingValue) || same(Minter,missingValue))
            values[i] = missingValue;
        else
            values[i] = derived.calculate(Kinter,Linter,Minter);
    }

    if(DEBUG)
        cout << " AverageWeightedRegular::derivedSubgridParameters END <--- " << endl;
}

double AverageWeightedRegular::interpolatedValue(const Point& where, const vector<FieldPoint>& nearests) const {
    throw NotImplementedFeature("AverageWeightedRegular::interpolatedValue");
    return 0;
}

void AverageWeightedRegular::print(ostream& out) const {
    Interpolator::print(out);
    out << "AverageWeightedRegular";
}
