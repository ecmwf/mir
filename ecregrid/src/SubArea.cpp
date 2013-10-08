/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "SubArea.h"

#ifndef Exception_H
#include "Exception.h"
#endif

#ifndef GridField_H
#include "GridField.h"
#endif

#ifndef Grid_H
#include "Grid.h"
#endif

#ifndef Point_H
#include "Point.h"
#endif

#ifndef GridContext_H
#include "GridContext.h"
#endif

SubArea::SubArea(double north, double west, double south, double east) :
	area_(north, west, south, east)
{
}

SubArea::SubArea(const Area& area) :
	area_(area)
{
}

SubArea::~SubArea()
{
}
/*
void SubArea::extract(const GridField& input, double* values, unsigned long valuesSize) const
{
//	long size = inputPoints.size();
//	cout << "SubArea::extract input size " << size << endl;
	cout << "SubArea::extract out size " << valuesSize << endl;

	const double*  inputData = input.data();
	long count = 0;

	auto_ptr<GridContext> ctx(input.grid().getGridContext());
	int westLongitudeIndex = 0, eastLongitudeIndex = 0;

	vector<long> gridSpec;
	size_t size_lats = input.grid().getGridDefinition(gridSpec);

	int northLatitudeIndex = input.grid().northIndex(area_.north());
	int southLatitudeIndex = input.grid().southIndex(area_.south());

	cout << "SubArea::extract northLatitudeIndex " << northLatitudeIndex << " southLatitudeIndex " << southLatitudeIndex << endl;

	double west = area_.west();
	double east = area_.east();

	long prev = 0;
	bool wrap = false;
	if( west < 0){
		wrap = true;
		west += 360.0;
	}
			cout << "SubArea::extract wrap " << wrap << " west " << west << endl;

	for ( size_t j = northLatitudeIndex ; j <= southLatitudeIndex ; j++ ) {
		if(gridSpec[j] != prev){
			input.grid().findWestAndEastIndex(ctx.get(), j, west, east, westLongitudeIndex, eastLongitudeIndex);
			prev = gridSpec[j];
			cout << "SubArea::extract " << j <<  " NUMBER of points : " << gridSpec[j] << "  west Index " << westLongitudeIndex << " east Index " << eastLongitudeIndex << endl;

		}
		if(wrap){
			for ( size_t i = westLongitudeIndex ; i < prev; i++ ) {
				long dataIndex = input.grid().getIndex(i,j);	
				values[count++] = inputData[dataIndex];
			}
			for ( size_t i = 0 ; i <= eastLongitudeIndex ; i++ ) {
				long dataIndex = input.grid().getIndex(i,j);	
				values[count++] = inputData[dataIndex];
			}
		}
		else{
			for ( size_t i = westLongitudeIndex ; i <= eastLongitudeIndex ; i++ ) {
				long dataIndex = input.grid().getIndex(i,j);	
				values[count++] = inputData[dataIndex];
			}
		}
	}

	cout << "SubArea::extract calculated: " << valuesSize << " counted: " << count << endl;

	ASSERT(valuesSize == count);

}
*/

void SubArea::extract(const GridField& input, vector<double>& values) const
{
    size_t valuesSize = values.size();

	const vector<double>&  inputData = input.data();
	long count = 0;

	auto_ptr<GridContext> ctx(input.grid().getGridContext());

	vector<long> gridSpec;
    /*size_t size_lats = */ input.grid().getGridDefinition(gridSpec);


	int northLatitudeIndex = input.grid().northIndex(area_.north());
	int southLatitudeIndex = input.grid().southIndex(area_.south());

	double west = area_.west();
	double east = area_.east();

	long prev = 0;

    // condition west and east so they are both
    // greater than zero as required by code below

    while (west < 0)
        west+=360;

    while (east < 0)
        east+=360;

	ASSERT(west >= 0.0);
    ASSERT(east >= 0.0);

    // we wrap if, with both values > 0, east is not greater than west
    bool wrap = (east < west);

	if(DEBUG)
		cout << "SubArea::extract wrap " << wrap << " west " << west << ", east = " << east << endl;

	double increment = 0;

    for ( int j = northLatitudeIndex ; j <= southLatitudeIndex ; j++ ) {
		if(gridSpec[j] != prev){
			prev = gridSpec[j];
			increment = 360.0 / prev;
		}
		double longitude = 0;
		if(wrap){
            // do the west --> zero chunk first
		    longitude = 0;
			for ( int i = 0 ; i < prev; i++ ) {                
                if(((longitude > west || same(longitude,west)) && longitude < 360.0))
                {
		            long dataIndex = input.grid().getIndex(i,j);    
		            values[count++] = inputData[dataIndex];
				 }

				 longitude += increment;
                        
            }
            // now fill in the zero --> east chunk
		    longitude = 0;
			for ( int i = 0 ; i < prev; i++ ) {
				if( ( (longitude > 0 || iszero(longitude)) && (longitude < east || same(longitude,east))))
                {                    
		            long dataIndex = input.grid().getIndex(i,j);    
		            values[count++] = inputData[dataIndex];
				 }
				
                 longitude += increment;
			}
		}
		else{
			for(int i = 0 ; i < prev ; i++) {
			    if((longitude > west || same(longitude,west)) && (longitude < east || same(longitude,east))){
		         	long dataIndex = input.grid().getIndex(i,j);    
		         	values[count++] = inputData[dataIndex];
				 }
				longitude += increment;
			}

		}
	}

	if(DEBUG)
		cout << "SubArea::extract calculated: " << valuesSize << " counted: " << count << endl;

	ASSERT((int)valuesSize == count);
}


void SubArea::print(ostream& out) const
{
	out << "SubArea{ Area=[" << area_ << "] }";
}
