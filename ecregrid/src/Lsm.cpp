/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "Lsm.h"

#ifndef Exception_H
#include "Exception.h"
#endif

#ifndef Point_H
#include "Point.h"
#endif

#ifndef GridField_H
#include "GridField.h"
#endif

#ifndef LsmPreDefined_H
#include "LsmPreDefined.h"
#endif

#ifndef Grid_H
#include "Grid.h"
#endif

#ifndef Input_H
#include "Input.h"
#endif

#ifndef GribApiOutputGrid_H
#include "GribApiOutputGrid.h"
#endif

#include <sys/stat.h>

/*static*/ ThreadSafeMap<std::vector<double> > Lsm::cache_;

Lsm::Lsm(Input* input) :
	input_(input)
{
	pathForGeneratedFiles_ = getDataDir();
	pathForGeneratedFiles_.append("/land_sea_masks");
	path_ = getShareDir();
	path_.append("/lsm");
	if(DEBUG){
		cout << "Lsm::Lsm Static files path: " << path_ << endl;
		cout << "Lsm::Lsm Generated files path: " << pathForGeneratedFiles_ << endl;
	}
}

Lsm::~Lsm()
{
}

ref_counted_ptr< const vector<double> > Lsm::getLsmValuesDouble(const Grid& gridSpec)
{
	if(isAvailablePredefinedLsm()){
		if(DEBUG)
			cout << "	^^^ Lsm::getLsmValues => PreDefined LSM is available :-(^^^" << endl;
		LsmPreDefined l(input_->newInput(input_->fileName()), directoryOfPredefined());
        return l.getLsmValuesDouble(gridSpec);
	}

	vector<Point> grid;
	gridSpec.generateGrid1D(grid);

	long size = grid.size();
	if(DEBUG)
		cout << "Lsm::getLsmValues grid.size() " << size << endl;

    // make a new one and save as a predefined
    vector<double>* values = new vector<double>(size);

    for ( int j = 0 ; j < size ;  j++ ) {
//		cout << "Lsm::getLsmValues grid.[j].latitude() " << grid[j].latitude() <<  "  " << grid[j].longitude() << " j " << j << endl;
		(*values)[j] = seaPoint(grid[j].latitude(),grid[j].longitude());
	}

	createGlobalLsmAndWriteToFileAsGrib(gridSpec);

    // cache these values
    return cache_.addItem(Lsm::cacheKey(*input_, gridSpec), values);

}

void Lsm::createGlobalLsmAndWriteToFileAsGrib(const Grid& gridSpec) const
{
	Grid*  globalGrid = gridSpec.getGlobalGrid();

	vector<Point> grid;
	globalGrid->generateGrid1D(grid);

	size_t size = grid.size();
    vector<double> generatedLsm(size);

    for ( size_t j = 0 ; j < size ;  j++ ){
		generatedLsm[j] = seaPoint(grid[j].latitude(),grid[j].longitude());
		}

	string levelType = "sfc";
// Scanning mode always north-south , west-east
	int scanMode = 1;
	int bitsPerValue = 12;
	int dateLsm = 20100401;
	auto_ptr<GridField> f ( new GridField(globalGrid,Parameter(172,128,levelType),"(0 - 1)",1,98,levelType,0,dateLsm,0,"h",0,0,bitsPerValue,scanMode,false, generatedLsm,MISSING_VALUE));

	GribApiOutputGrid o(directoryOfPredefined() + input_->fileName());
	if(DEBUG)
		cout << "Lsm::createGlobalLsmAndWriteToFileAsGrib - file name: " << directoryOfPredefined() + input_->fileName() << endl;
	o.write(*f);
	if(chmod( (directoryOfPredefined() + input_->fileName()).c_str(), (mode_t) 0444 ))
	        throw CantOpenFile("Lsm::createWholeGlobeAndWriteToFile cannot change mode to 444" + input_->fileName());
}

/*static*/ string Lsm::cacheKey(const Input& in, const Grid& grid)
{
    // Generate from input and grid area
    // - other values may be added to generate sufficiently unique keys as required
    stringstream ss;
    ss << in.fileName() << "_" << grid.north() << "_" << grid.south() << "_" << grid.east() << "_" << grid.west();
    return ss.str();
}
