/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "FluxConservingReduced.h"

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
#include <eckit/log/Timer.h>
#endif

#ifndef GridContext_H
#include "GridContext.h"
#endif

#ifndef DerivedSubgridParameters_H
#include "DerivedSubgridParameters.h"
#endif

#include <algorithm>

FluxConservingReduced::FluxConservingReduced(const Grid &in, const Grid &out) :
    Interpolator(64), northSouthNumberOfPoints_(out.northSouthNumberOfPoints()), northSouthIncrement_(out.northSouthIncrement()), westEastGlobal_(out.isGlobalWestEast()), east_(out.east()), west_(out.west()) {
    sizeSpec_ = out.getGridDefinition(gridSpec_);
    ASSERT(northSouthNumberOfPoints_ == (int)sizeSpec_);

    size_t length = in.calculatedNumberOfPoints();
    cellArea_.reserve(length);
    cellAreaSize_.reserve(length);
    in.cellsAreas(cellArea_, cellAreaSize_);
}

FluxConservingReduced::~FluxConservingReduced() {
    //cout << "FluxConservingReduced: cleaning up." << endl;
}

void FluxConservingReduced::interpolate(const Grid &input, const double *data, int inScMode, double missingValue, const vector<Point> &outputPoints, double *values, unsigned long valuesSize) const {
    auto_ptr<GridContext> ctx(input.getGridContext());

    if (DEBUG)
        cout << " FluxConservingReduced::interpolate START ---> " << endl;

    unsigned long count = 0;
    for (size_t j = 0 ; j < sizeSpec_ ; j++) {
        long numPts = gridSpec_[j];
        if (!numPts)
            continue;
        double westEastInc =  westEastIncrement(numPts);
        //			cout << "reduced numPts: " << numPts << endl;
        for (int i = 0 ; i < numPts ; i++) {
            values[count] = input.fluxConserving(ctx.get(), outputPoints[count], cellArea_, cellAreaSize_, data, inScMode, missingValue, northSouthIncrement_, westEastInc);
            count++;
        }
    }

    if (DEBUG)
        cout << " FluxConservingReduced::interpolate END <--- " << endl;

    ASSERT(valuesSize == count);
}

void FluxConservingReduced::standardDeviation(const Grid &input, const double *data, const double *dataSquared, int inScMode, double missingValue, const vector<Point> &outputPoints, double *values, unsigned long valuesSize) const {
    auto_ptr<GridContext> ctx(input.getGridContext());

    if (DEBUG)
        cout << " FluxConservingReduced::standardDeviation START ---> " << endl;

    unsigned long count = 0;
    for (size_t j = 0 ; j < sizeSpec_ ; j++) {
        long numPts = gridSpec_[j];
        if (!numPts)
            continue;
        double westEastInc =  westEastIncrement(numPts);
        for (int i = 0 ; i < numPts ; i++) {
            double Linter = input.fluxConserving(ctx.get(), outputPoints[count], cellArea_, cellAreaSize_, data, inScMode, missingValue, northSouthIncrement_, westEastInc);
            double Kinter = input.fluxConserving(ctx.get(), outputPoints[count], cellArea_, cellAreaSize_, dataSquared, inScMode, missingValue, northSouthIncrement_, westEastInc);

            if (same(Linter, missingValue) || same(Kinter, missingValue))
                values[count] = missingValue;
            else
                values[count] = sqrt(max(0.0, (Kinter - Linter * Linter))) ;

            count++;
        }
    }

    if (DEBUG)
        cout << " FluxConservingReduced::standardDeviation END <--- " << endl;
}

void FluxConservingReduced::derivedSubgridParameters(const Grid &input, const double *K, const double *L, const double *M, int inScMode, double missingValue, const vector<Point> &outputPoints, double *values, unsigned long valuesSize, const DerivedSubgridParameters &stat) const {
    vector<FieldPoint> nearests;
    nearests.reserve(neighbour_);

    auto_ptr<GridContext> ctx(input.getGridContext());

    if (DEBUG)
        cout << " FluxConservingReduced::derivedSubgridParameters START ---> " << stat << endl;
    unsigned long count = 0;
    for (size_t j = 0 ; j < sizeSpec_ ; j++) {
        long numPts = gridSpec_[j];
        if (!numPts)
            continue;
        double westEastInc = westEastIncrement(numPts);
        for (long i = 0 ; i < numPts ; i++) {
            double Kinter = input.fluxConserving(ctx.get(), outputPoints[count], cellArea_, cellAreaSize_, K, inScMode, missingValue, northSouthIncrement_, westEastInc);
            double Linter = input.fluxConserving(ctx.get(), outputPoints[count], cellArea_, cellAreaSize_, L, inScMode, missingValue, northSouthIncrement_, westEastInc);
            double Minter = input.fluxConserving(ctx.get(), outputPoints[count], cellArea_, cellAreaSize_, M, inScMode, missingValue, northSouthIncrement_, westEastInc);

            if (same(Linter, missingValue) || same(Kinter, missingValue) || same(Minter, missingValue))
                values[count] = missingValue;
            else
                values[count] = stat.calculate(Kinter, Linter, Minter);

            count++;
        }
    }

    if (DEBUG)
        cout << " FluxConservingReduced::derivedSubgridParameters END <--- " << endl;
}

double FluxConservingReduced::interpolatedValue(const Point &where, const vector<FieldPoint> &nearests) const {
    throw NotImplementedFeature("FluxConservingReduced::interpolatedValue");
    return 0;
}

void FluxConservingReduced::print(ostream &out) const {
    Interpolator::print(out);
    out << "FluxConservingReduced";
}
