/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "Frame.h"

#ifndef Exception_H
#include "Exception.h"
#endif

#ifndef GridField_H
#include "GridField.h"
#endif

#ifndef Grid_H
#include "Grid.h"
#endif

Frame::Frame(int number, double missingValue) :
	Extraction(), numberOfPointsAcrossTheFrame_(number), missingValue_(missingValue)
{
	if(DEBUG)
		cout << "Frame::Frame " << number << endl;;
}
	

Frame::~Frame()
{
}

void Frame::extract(const Grid& output, vector<double>& values) const
{
    const size_t valuesSize = values.size();

	int northSouthNumberOfPoints = output.northSouthNumberOfPoints();
	int westEastNumberOfPoints   = output.westEastNumberOfPoints();

	ASSERT(westEastNumberOfPoints * northSouthNumberOfPoints == (long)valuesSize);
	if(DEBUG){
		cout << "------------------------------------------------------------------" << endl;
		cout << "Frame::extract westEastNumberOfPoints*northSouthNumberOfPoints " << westEastNumberOfPoints*northSouthNumberOfPoints << " valuesSize " << valuesSize <<  endl;
		cout << "Number Of Points Across The Frame: " << numberOfPointsAcrossTheFrame_ << endl;
		cout << "------------------------------------------------------------------" << endl;
	}

	int top    = numberOfPointsAcrossTheFrame_;
	int bottom = northSouthNumberOfPoints - numberOfPointsAcrossTheFrame_;
	int left   = numberOfPointsAcrossTheFrame_;
	int right  = westEastNumberOfPoints - numberOfPointsAcrossTheFrame_;

	long next = 0;
    for (int i = top ; i < bottom ; i++) {
		next = i * westEastNumberOfPoints + left - 1;
    	for (int j = left ; j < right ; j++) {
			++next;
			values[next] = missingValue_;
		}
	}
}

void Frame::print(ostream& out) const
{
	out << "Frame{ Number of points across the Frame =[" << numberOfPointsAcrossTheFrame_ << "] }";
}
