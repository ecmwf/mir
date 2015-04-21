/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "AverageWeightedReducedLsm.h"

#include "Exception.h"
#include "FieldPoint.h"
#include "Grid.h"
#include "GridField.h"
#include <eckit/utils/Timer.h>
#include "GridContext.h"
#include "DerivedSubgridParameters.h"

#include <algorithm>

AverageWeightedReducedLsm::AverageWeightedReducedLsm(const Grid& in, const Grid& out, const string& lsmMethod) :
    InterpolatorLsm(in,out,lsmMethod), northSouthNumberOfPoints_(out.northSouthNumberOfPoints()), northSouthIncrement_(out.northSouthIncrement()), westEastGlobal_(out.isGlobalWestEast()), east_(out.east()), west_(out.west()) {
    sizeSpec_ = out.getGridDefinition(gridSpec_);
    ASSERT(northSouthNumberOfPoints_ == (int)sizeSpec_);

    weights_.reserve(in.calculatedNumberOfPoints());
    in.aWeights(weights_);
    out.latitudes(outLats_);
}

AverageWeightedReducedLsm::~AverageWeightedReducedLsm() {
//cout << "AverageWeightedReducedLsm: cleaning up." << endl;
}

void AverageWeightedReducedLsm::interpolate(const Grid& input, const vector<double>& data, int inScMode, double missingValue, const vector<Point>& outputPoints, vector<double>& values) const {
    auto_ptr<GridContext> ctx(input.getGridContext());

    if(DEBUG)
        cout << " AverageWeightedReducedLsm::interpolate START ---> " << endl;

    unsigned long count = 0;
    for (size_t j = 0 ; j < sizeSpec_ ; j++) {
        long numPts = gridSpec_[j];
        if(!numPts)
            continue;
        double westEastInc =  westEastIncrement(numPts);
//			cout << "reduced numPts: " << numPts << endl;
        // NB we can pass dereferenced lsm data ojects here as we hold ref_counted_ptr for
        // them as class members. hence they are guaranteed to exist while this
        // call completes
        for (int i = 0 ; i < numPts ; i++) {
            values[count] = input.averageWeightedLsm(ctx.get(),outputPoints[count],weights_,data,*inLsmData_,*outLsmData_,inScMode,missingValue,outLats_,westEastInc);
            count++;
        }
    }

    if(DEBUG)
        cout << " AverageWeightedReducedLsm::interpolate END <--- " << endl;

    ASSERT(values.size() == count);
}

void AverageWeightedReducedLsm::standardDeviation(const Grid& input, const vector<double>& data, const vector<double>& dataSquared, int inScMode, double missingValue, const vector<Point>& outputPoints, vector<double>& values) const {
    auto_ptr<GridContext> ctx(input.getGridContext());

    if(DEBUG)
        cout << " AverageWeightedReducedLsm::standardDeviation START ---> " << endl;

    unsigned long count = 0;
    for (size_t j = 0 ; j < sizeSpec_ ; j++) {
        long numPts = gridSpec_[j];
        if(!numPts)
            continue;
        double westEastInc =  westEastIncrement(numPts);
        for (int i = 0 ; i < numPts ; i++) {
            // NB we can pass dereferenced lsm data ojects here as we hold ref_counted_ptr for
            // them as class members. hence they are guaranteed to exist while this
            // call completes
            double Linter = input.averageWeightedLsm(ctx.get(),outputPoints[count],weights_,data,*inLsmData_,*outLsmData_,inScMode,missingValue,outLats_,westEastInc);
            double Kinter = input.averageWeightedLsm(ctx.get(),outputPoints[count],weights_,dataSquared,*inLsmData_,*outLsmData_,inScMode,missingValue,outLats_,westEastInc);

            if(same(Linter,missingValue) || same(Kinter,missingValue))
                values[count] = missingValue;
            else
                values[count] = sqrt(max(0.0,(Kinter - Linter*Linter))) ;

            count++;
        }
    }

    if(DEBUG)
        cout << " AverageWeightedReducedLsm::standardDeviation END <--- " << endl;
}

void AverageWeightedReducedLsm::derivedSubgridParameters(const Grid& input, const vector<double>& K, const vector<double>& L, const vector<double>& M, int inScMode, double missingValue, const vector<Point>& outputPoints, vector<double>& values, const DerivedSubgridParameters& stat) const {
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
        for (int i = 0 ; i < numPts ; i++) {
            // NB we can pass dereferenced lsm data ojects here as we hold ref_counted_ptr for
            // them as class members. hence they are guaranteed to exist while this
            // call completes
            double Kinter = input.averageWeightedLsm(ctx.get(),outputPoints[count],weights_,K,*inLsmData_,*outLsmData_,inScMode,missingValue,outLats_,westEastInc);
            double Linter = input.averageWeightedLsm(ctx.get(),outputPoints[count],weights_,L,*inLsmData_,*outLsmData_,inScMode,missingValue,outLats_,westEastInc);
            double Minter = input.averageWeightedLsm(ctx.get(),outputPoints[count],weights_,M,*inLsmData_,*outLsmData_,inScMode,missingValue,outLats_,westEastInc);

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

double AverageWeightedReducedLsm::interpolatedValue(const Point& where, const vector<FieldPoint>& nearests) const {
    throw NotImplementedFeature("AverageWeightedReducedLsm::interpolatedValue");
    return 0;
}

void AverageWeightedReducedLsm::print(ostream& out) const {
    Interpolator::print(out);
    out << "AverageWeightedReducedLsm";
}
