/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "LsmLatLon1km.h"

#ifndef Grid_H
#include "Grid.h"
#endif

#ifndef RegularLatLon_H
#include "RegularLatLon.h"
#endif

#ifndef LsmPreDefined_H
#include "LsmPreDefined.h"
#endif

LsmLatLon1km::LsmLatLon1km() 
{
}

LsmLatLon1km::~LsmLatLon1km()
{
	delete [] llData_;
}

bool* LsmLatLon1km::getLsmValues( const Grid& gridSpec )
{
    if(DEBUG)
        cout << "LsmPreDefined::getLsmValues ----- PREDEFINED ----- " << path_ + directo
ryOfPredefined_ + input_->fileName() << endl;

    if(isAvailablePredefinedLsm()){
		LsmPreDefined l(input_->newInput(input_->fileName()), directoryOfPredefined());
        return l.getLsmValues(gridSpec);
    }
	//180/21600 == .00833333333333333333
	RegularLatLon ll(180/21600,180/21600);

    size_t llSizeFromGrib;

    llData_ = input_->getDoubleValues(path_, &llSizeFromGrib);

	long llSizeCalclulated = ll.calculatedNumberOfPoints();
    ASSERT(llSizeCalclulated == llSizeFromGrib);

    auto_ptr<Grid>globalGrid(gridSpec.getGlobalGrid());
    long globalGridSizeCalclulated = globalGrid->calculatedNumberOfPoints();

    if(DEBUG)
        cout << "LsmPreDefined::getLsmValues -- globalGridSizeCalclulated " << globalGridSizeCalclulated << endl;


    vector<Point> gridRequired;
    gridSpec.generateGrid1D(gridRequired,0);
    long requiredGridSize = gridRequired.size();


    bool* generatedLsm  = new bool[requiredGridSize];

    if(requiredGridSize == globalGridSize) {
        if(DEBUG)
            if(llData_)
                cout << "LsmPreDefined::getLsmValues Global -- requiredGridSize " << req
uiredGridSize << endl;
        for ( int i = 0 ; i < requiredGridSize ;  i++ ) {
//            if (llData_[i] >= 0.5)
// Nils
            if (llData_[i] > 0.5)
                generatedLsm[i] = true;
            else
                generatedLsm[i] = false;
        }
        return generatedLsm;
    }
}

double LsmLatLon1km::seaPoint(double lat, double lon) const
{
	Point pp(lat,lon);
    int nNumber = 4;
    nearests.reserve(nNumber);

    auto_ptr<GridContext> ctx(globalGrid->getGridContext());
//ssp  scanMode is always 1 for lsm predefined
    int scanMode = 1;

        globalGrid->nearestPoints(ctx.get(),gridRequired[i],nearests,llData_,scanMode,nNumber);
        return  nn.interpolatedValue(gridRequired[i],nearests);

}

bool LsmLatLon1km::isAvailablePredefinedLsm()
{
    if(DEBUG)
        cout << "LsmLatLon1km::isAvailablePredefinedLsm " << path_ + directoryOfPredefined_ << endl;

    if(input_->exist(path_ + directoryOfPredefined_))
        return true;
	return false;
}

void LsmLatLon1km::print(ostream&) const
{
}
