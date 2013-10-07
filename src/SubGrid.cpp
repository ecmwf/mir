/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "SubGrid.h"

#ifndef Exception_H
#include "Exception.h"
#endif

#ifndef GridField_H
#include "GridField.h"
#endif

#ifndef Grid_H
#include "Grid.h"
#endif

#ifndef RegularLatLon_H
#include "RegularLatLon.h"
#endif

SubGrid::SubGrid(double ns, double we) :
	Extraction(), ns_(ns), we_(we)
{
}

SubGrid::~SubGrid()
{
}

GridField* SubGrid::extract(GridField& field) const
{
	string name = field.nameOfField(); 
	if(name != "regular_ll")
		throw UserError("SubGrid::extract -- SubGrid Can be extract only from Regular LatLon Field. Required Grid to be extracted is: " + name);

	int northSouthNumberOfPoints = field.grid().northSouthNumberOfPoints();
    int westEastNumberOfPoints   = field.grid().westEastNumberOfPoints();
    long dataSize                = field.dataLength();

	ASSERT(westEastNumberOfPoints*northSouthNumberOfPoints == dataSize);
	const vector<double>& data = field.data();

	double northSouthIncrement = field.grid().northSouthIncrement();
	double westEastIncrement   = field.grid().westEastIncrement();

	
	int factorNS = northSouthIncrement / ns_;
	if(northSouthIncrement != ns_*factorNS)
		throw UserError("SubGrid::extract -- Output north-south step not multiple of input ");

	int factorWE = westEastIncrement / we_;
	if(westEastIncrement != we_*factorWE)
		throw UserError("SubGrid::extract -- Output west-east step not multiple of input ");
	
	double north = field.grid().north();
	double south = field.grid().south();
	double west  = field.grid().west();
	double east  = field.grid().east();

	int skipedNumberNS = 0;
	if(northSouthNumberOfPoints % factorNS)
		skipedNumberNS = northSouthNumberOfPoints / factorNS + 1;
	else {
		skipedNumberNS = northSouthNumberOfPoints / factorNS;
		south = south - northSouthIncrement;
	}

	int skipedNumberWE = 0;
	if(westEastNumberOfPoints % factorWE)
		skipedNumberWE = westEastNumberOfPoints / factorWE + 1;
	else {
		skipedNumberWE = westEastNumberOfPoints / factorWE; 
		east = east - westEastIncrement;
	}

	long valuesSize = skipedNumberNS * skipedNumberWE;
    vector<double> values(valuesSize);
    std::copy(data.begin(), data.end(), values.begin());

//	cout << "SubGrid::extract westEastNumberOfPoints*northSouthNumberOfPoints " << westEastNumberOfPoints*northSouthNumberOfPoints << " valuesSize " << valuesSize <<  endl;

	long next = 0, count = 0;
    for (int i = 0 ; i < northSouthNumberOfPoints ; i += factorNS) {
		next = i * westEastNumberOfPoints;
    	for (int j = 0 ; j < westEastNumberOfPoints ; j +=factorWE)
			values[count++] = data[next+j];
	}

	RegularLatLon* llgrid = new RegularLatLon(north, west, south, east, ns_, we_);

	return new GridField(llgrid,field,field.bitsPerValue(),field.editionNumber(),field.scanningMode(),field.bitmap(),values,field.missingValue());
}


void SubGrid::print(ostream& out) const
{
	out << "SubGrid{ NS increment=[" << ns_ << "], WE increment=[" << we_ << "] }";
}
