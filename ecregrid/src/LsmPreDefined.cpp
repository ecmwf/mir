/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "LsmPreDefined.h"

#ifndef Exception_H
#include "Exception.h"
#endif

#ifndef Grid_H
#include "Grid.h"
#endif

#ifndef GridContext_H
#include "GridContext.h"
#endif


#ifndef LsmEmos10minute_H
#include "LsmEmos10minute.h"
#endif

#ifndef Point_H
#include "Point.h"
#endif

#ifndef FieldPoint_H
#include "FieldPoint.h"
#endif

#ifndef NearestNeigbour_H
#include "NearestNeigbour.h"
#endif

#ifndef Input_H
#include "Input.h"
#endif

LsmPreDefined::LsmPreDefined(Input* input):
	Lsm(input), directoryOfPredefined_(path_+"/predefined/")
{
}

LsmPreDefined::LsmPreDefined(Input* input, const string& predefined):
	Lsm(input), directoryOfPredefined_(predefined)
{
}

LsmPreDefined::~LsmPreDefined()
{
}

bool LsmPreDefined::isAvailablePredefinedLsm()
{
	return input_->exist(directoryOfPredefined_);
}

double LsmPreDefined::seaPoint(double lat, double lon) const
{
	throw NotImplementedFeature("LsmPreDefined::seaPoint()");
}

bool LsmPreDefined::seaPointBool(double lat, double lon) const
{
	throw NotImplementedFeature("LsmPreDefined::seaPoint()");
}

ref_counted_ptr< const vector<double> > LsmPreDefined::getLsmValuesDouble(const Grid& gridSpec)
{
	if(!isAvailablePredefinedLsm()){
        throw UserError("LsmPreDefined::getLsmValuesDouble PREDEFINED is NOT available-> ", directoryOfPredefined_ + input_->fileName());
	}

	if(DEBUG)
		cout << "LsmPreDefined::getLsmValuesDouble ----- PREDEFINED ----- " << directoryOfPredefined_ + input_->fileName() << endl;

    // first check the cache

    string key = Lsm::cacheKey(*input_, gridSpec);

    ref_counted_ptr< const vector<double> > vals = Lsm::cache_.getItem(key);
    if (vals.get())
        return vals;

    // else we load the values and then cache them

	auto_ptr<Grid>lsmPredef(input_->defineGridForCheck(directoryOfPredefined_));

    vector<double>* wholeLsm = new vector<double>();
    input_->getDoubleValues(directoryOfPredefined_, *wholeLsm);
    const size_t lsmSize = (*wholeLsm).size();

	if(*lsmPredef == gridSpec) {
		if(DEBUG)
			cout << "LsmPreDefined::getLsmValuesDouble -- Required and Predefined Lsm are the Same return  = > size " << lsmSize << endl;

        // add to cache and return ref counted pointer:
        return Lsm::cache_.addItem(key, wholeLsm);
	}

	if(DEBUG){
		cout << "LsmPreDefined::getLsmValuesDouble OUTPUT is NOT GLOBAL" << endl;
		cout << "LsmPreDefined::getLsmValuesDouble lsmPredef " << *lsmPredef << endl;
		cout << "LsmPreDefined::getLsmValuesDouble lsmPredef " << gridSpec << endl;
	}

	unsigned long requiredGridSizeCalculated = gridSpec.calculatedNumberOfPoints();
	vector<Point> gridRequired;
	gridSpec.generateGrid1D(gridRequired);
    size_t requiredGridSize = gridRequired.size();

	ASSERT(requiredGridSize == requiredGridSizeCalculated);

    vector<double>* generatedLsm = new vector<double>(requiredGridSize);

	int nNumber = 4;
	NearestNeigbour nn;
	vector<FieldPoint> nearests;
	nearests.reserve(nNumber);

	auto_ptr<GridContext> ctx(gridSpec.getGridContext());
	if(DEBUG)
		cout << "LsmPreDefined::getLsmValuesDouble -- Extract Subarea with NN --- " << endl;
	int scanMode = 1;
    for ( size_t i = 0 ; i < requiredGridSize ;  i++ ) {
		lsmPredef->nearestPoints(ctx.get(),gridRequired[i],nearests,*wholeLsm,scanMode,nNumber);
		(*generatedLsm)[i] = nn.interpolatedValue(gridRequired[i],nearests);
	}

    delete wholeLsm;
    return Lsm::cache_.addItem(key, generatedLsm);
}

void LsmPreDefined::getLsmValues( const Grid& gridSpec, vector<bool>& generatedLsm)
{
	bool emosLsm = false;
#if ECREGRID_EMOS_SIMULATION
		emosLsm = true;
#endif

	if(DEBUG)
		cout << "LsmPreDefined::getLsmValues ----- PREDEFINED ----- " << directoryOfPredefined_ + input_->fileName() << endl;

	if(!isAvailablePredefinedLsm()){
        throw UserError("LsmPreDefined::getLsmValues PREDEFINED is NOT available-> ", directoryOfPredefined_ + input_->fileName());
	}

	vector<double> wholeGlobe;
    input_->getDoubleValues(directoryOfPredefined_, wholeGlobe);
	const size_t globalGridSize = wholeGlobe.size();
	auto_ptr<Grid>globalGrid(gridSpec.getGlobalGrid());

	unsigned long globalGridSizeCalculated = globalGrid->calculatedNumberOfPoints();

	if(DEBUG)
		cout << "LsmPreDefined::getLsmValues -- globalGridSize " << globalGridSize << " globalGridSizeCalculated " << globalGridSizeCalculated << endl;

	ASSERT(globalGridSizeCalculated == globalGridSize);

	vector<Point> gridRequired;
	gridSpec.generateGrid1D(gridRequired);
	size_t requiredGridSize = gridRequired.size();

	generatedLsm.clear();
	generatedLsm.reserve(requiredGridSize);

	if(requiredGridSize == globalGridSize) {
		if(DEBUG)
			if(wholeGlobe.size() > 0)
				cout << "LsmPreDefined::getLsmValues Global -- requiredGridSize " << requiredGridSize << endl;
    	for ( size_t i = 0 ; i < requiredGridSize ;  i++ ) {
        	if (wholeGlobe[i] > 0.5)
				generatedLsm.push_back(true);
			else if ( emosLsm && same(wholeGlobe[i],0.5))
				generatedLsm.push_back(true);
			else
				generatedLsm.push_back(false);
		}
		return;
	}
	if(DEBUG)
		cout << "LsmPreDefined::getLsmValues -- r	equiredGridSize " << requiredGridSize << endl;

	int nNumber = 4;
	NearestNeigbour nn;
	vector<FieldPoint> nearests;
	nearests.reserve(nNumber);

	auto_ptr<GridContext> ctx(globalGrid->getGridContext());

    for ( size_t  i = 0 ; i < requiredGridSize ;  i++ ) {
		globalGrid->nearestPoints(ctx.get(),gridRequired[i],nearests,wholeGlobe,0,nNumber);
		double value = nn.interpolatedValue(gridRequired[i],nearests);
// Nils > 0.5 
        if (value > 0.5)
			generatedLsm.push_back(true);
		else if ( emosLsm && same(value,0.5))
			generatedLsm.push_back(true);
		else
			generatedLsm.push_back(false);
	}
}

/*
void LsmPreDefined::print(ostream&) const
{
}
*/
