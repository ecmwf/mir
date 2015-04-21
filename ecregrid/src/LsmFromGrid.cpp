/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "LsmFromGrid.h"

#ifndef Exception_H
#include "Exception.h"
#endif

#ifndef Point_H
#include "Point.h"
#endif

#ifndef FieldPoint_H
#include "FieldPoint.h"
#endif

#ifndef Input_H
#include "Input.h"
#endif

#ifndef Grid_H
#include "Grid.h"
#endif

#ifndef GridField_H
#include "GridField.h"
#endif

#ifndef Interpolator_H
#include "Interpolator.h"
#endif

#ifndef LsmPreDefined_H
#include "LsmPreDefined.h"
#endif

#ifndef Parameter_H
#include "Parameter.h"
#endif

#ifndef GridContext_H
#include "GridContext.h"
#endif

#ifndef GribApiOutputGrid_H
#include "GribApiOutputGrid.h"
#endif

#include <sys/stat.h>

LsmFromGrid::LsmFromGrid(Input* input, Grid* grid, Interpolator* method) :
    Lsm( input), grid_(grid), method_(method), directoryOfPredefined_("/latlon1km") {
}

LsmFromGrid::~LsmFromGrid() {
}

long LsmFromGrid::value(double lat, double lon) const {
    throw NotImplementedFeature("LsmFromGrid::value()");
}

double LsmFromGrid::seaPoint(double lat, double lon) const {
    throw NotImplementedFeature("LsmFromGrid::seaPoint");
}

bool LsmFromGrid::seaPointBool(double lat, double lon) const {
    throw NotImplementedFeature("LsmFromGrid::seaPoint");
}

void LsmFromGrid::getLsmValues( const Grid& gridSpec, vector<bool>& generatedLsm ) {
    if(DEBUG)
        cout << "LsmPreDefined::getLsmValues ----- PREDEFINED ----- " << path_ + directoryOfPredefined_ + input_->fileName() << endl;

    if(isAvailablePredefinedLsm()) {
        LsmPreDefined l(input_->newInput(input_->fileName()), directoryOfPredefined());
        l.getLsmValues(gridSpec, generatedLsm);
        return;
    }
    //180/21600 == .00833333333333333333
    //RegularLatLon ll(180/21600,180/21600);

    vector<double> data;
    input_->getDoubleValues(path_, data);

    ASSERT(grid_->calculatedNumberOfPoints() == data.size());


//    if(DEBUG)
//       cout << "LsmPreDefined::getLsmValues -- globalGridSizeCalclulated " << globalGridSizeCalclulated << endl;


    vector<Point> gridRequired;
    gridSpec.generateGrid1D(gridRequired);

    size_t requiredGridSize = gridRequired.size();

    auto_ptr<GridContext> ctx(grid_->getGridContext());

    generatedLsm.resize(requiredGridSize);
    vector<FieldPoint> nearests;

// ssp supposed to be
    int inScMode = 1;

    for (size_t i = 0 ; i < requiredGridSize ; i++) {
        grid_->nearestPoints(ctx.get(),gridRequired[i],nearests,data,inScMode,method_->numberOfNeighbours());
        double temp = method_->interpolatedValue(gridRequired[i],nearests);
        if (temp > 0.5)
            generatedLsm[i] = true;
        else
            generatedLsm[i] = false;
    }

    createGlobalLsmAndWriteToFileAsGrib(gridSpec, data);

}

void LsmFromGrid::createGlobalLsmAndWriteToFileAsGrib(const Grid& gridSpec, const vector<double>& data) const {
    Grid* globalGrid = gridSpec.getGlobalGrid();
    long globalGridSizeCalclulated = globalGrid->calculatedNumberOfPoints();

    vector<Point> gridRequired;
    globalGrid->generateGrid1D(gridRequired);
    vector<FieldPoint> nearests;

    vector<double> generatedLsm(globalGridSizeCalclulated);
    auto_ptr<GridContext> ctx(globalGrid->getGridContext());

// ssp supposed to be
    int inScMode = 1;

    for (int i = 0 ; i < globalGridSizeCalclulated ; i++) {
        grid_->nearestPoints(ctx.get(),gridRequired[i],nearests,data,inScMode,method_->numberOfNeighbours());
        generatedLsm[i] = method_->interpolatedValue(gridRequired[i],nearests);
    }


//////////////////////////////////////////////////////////////////////
    string levelType = "sfc";
// Scanning mode always north-south , west-east
    int scanMode = 1;
    int bitsPerValue = 12;
    int dateLsm = 20100401;
    auto_ptr<GridField> f ( new GridField(globalGrid,Parameter(172,128,levelType),"(0 - 1)",1,98,levelType,0,dateLsm,0,"h",0,0,bitsPerValue,scanMode,false, generatedLsm,MISSING_VALUE));

    GribApiOutputGrid o(path_ + directoryOfPredefined() + input_->fileName());
    if(DEBUG)
        cout << "LsmFromGrid::createGlobalLsmAndWriteToFileAsGrib - file name: " << path_ + directoryOfPredefined() + input_->fileName() << endl;
    o.write(*f);
    if(chmod( (path_ + directoryOfPredefined() + input_->fileName()).c_str(), (mode_t) 0444 ))
        throw CantOpenFile("LsmFromGrid::createWholeGlobeAndWriteToFile cannot change mode to 444" + input_->fileName());

//////////////////////////////////////////////////////////////////////

}


bool LsmFromGrid::isAvailablePredefinedLsm() {
    if(DEBUG)
        cout << "LsmFromGrid::isAvailablePredefinedLsm " << path_ + directoryOfPredefined_ << endl;

    if(input_->exist(path_ + directoryOfPredefined_))
        return true;
    return false;
}

void LsmFromGrid::print(ostream&) const {
}
