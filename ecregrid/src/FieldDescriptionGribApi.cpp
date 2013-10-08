/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "FieldDescriptionGribApi.h"

#ifndef Exception_H
#include "Exception.h"
#endif

#ifndef Factory_H
#include "Factory.h"
#endif

#ifndef GridField_H
#include "GridField.h"
#endif

#ifndef ListOfPoints_H
#include "ListOfPoints.h"
#endif

#ifndef GribApiInput_H
#include "GribApiInput.h"
#endif

#ifndef Transformer_H
#include "Transformer.h"
#endif

FieldDescriptionGribApi::FieldDescriptionGribApi() :
	FieldDescription()
{
}

FieldDescriptionGribApi::FieldDescriptionGribApi(grib_handle* handle) :
	FieldDescription()
{

	long   parameter1 = 0, scanningMode1 = 0, table = 0;
	double north = 0, west = 0, south = 0, east = 0;

	double weIncrement = 0, nsIncrement = 0;
	long editionNumber1 = 1;
//	long bitmapPresent = 0;
	long level1 = 0, date1 = 0, time1 = 0;
	long startStep1 = 0, endStep1 = 0;
//	long stepU1 = 0;
	long bitsPerValue1 = 0;
	long truncation1 = 0, gaussianNumber1 = 0;
	size_t rgSpecLength = 0;
	long* rgSpec = 0;

	long centre1 = 0;

	char levelType1[80] ;
	char stepUnits1[80] ;
	char gridType[80];
	char units1[180] ;

	size_t size = sizeof(levelType1);
	size_t size1 = sizeof(units1);

 	GRIB_CHECK(grib_get_string(handle,"typeOfGrid",gridType,&size),0);
	string composedName(gridType);

	id_.setFromTypeOfGrid(composedName);

	GRIB_CHECK(grib_get_string(handle,"levelType",levelType1,&size),0);
	levelType(levelType1);
	GRIB_CHECK(grib_get_long(handle,"level",&level1),0);
	level(level1);
	GRIB_CHECK(grib_get_long(handle,"dataDate",&date1),0);
	date(date1);
	GRIB_CHECK(grib_get_long(handle,"dataTime",&time1),0);
	time(time1);
	GRIB_CHECK(grib_get_long(handle,"centre",&centre1),0);
	centre(centre1);
	GRIB_CHECK(grib_get_string(handle,"units",units1,&size1),0);
	units(units1);
	GRIB_CHECK(grib_get_string(handle,"stepUnits",stepUnits1,&size),0);
	stepUnits(stepUnits1);
	GRIB_CHECK(grib_get_long(handle,"startStep",&startStep1),0);
	startStep(startStep1);
	GRIB_CHECK(grib_get_long(handle,"endStep",&endStep1),0);
	endStep(endStep1);
	GRIB_CHECK(grib_get_long(handle,"bitsPerValue",&bitsPerValue1),0);
	bitsPerValue(bitsPerValue1);

	GRIB_CHECK(grib_get_long(handle,"editionNumber",&editionNumber1),0);
	editionNumber(editionNumber1);
	GRIB_CHECK(grib_get_long(handle,"paramId",&parameter1),0);

	if(DEBUG){
		cout << "FieldDescriptionGribApi -- Edition Number = " << editionNumber1 << endl;
		cout << "FieldDescriptionGribApi -- Type Of Grid = " << gridType << endl;
		cout << "FieldDescriptionGribApi -- Parameter = " << parameter1 << endl;
		cout << "FieldDescriptionGribApi -- Centre = " << centre1 << endl;
		cout << "FieldDescriptionGribApi -- Date = " << date1 << endl;
		cout << "FieldDescriptionGribApi -- Time = " << time1 << endl;
		cout << "FieldDescriptionGribApi -- Level Type = " << levelType1 << endl;
		cout << "FieldDescriptionGribApi -- Level = " << level1 << endl;
		cout << "FieldDescriptionGribApi -- Units = " << units1 << endl;
		cout << "FieldDescriptionGribApi -- Step Units = " << stepUnits1 << endl;
		cout << "FieldDescriptionGribApi -- Step Start = " << startStep1 << endl;
		cout << "FieldDescriptionGribApi -- Step End = " << endStep1 << endl;
		cout << "FieldDescriptionGribApi -- Bits Per Value = " <<  bitsPerValue1 << endl;
	}

	if(parameter1 > 1000) {
		table     = parameter1 / 1000;
		parameter1 = parameter1 % 1000;
	}
	else
		table = 128;

	Parameter param = Parameter(parameter1,table,levelType1);
	parameter(param);

	if(!id_.gridOrSpectral_) {
	// Spherical harmonics
		GRIB_CHECK(grib_get_long(handle,"pentagonalResolutionParameterJ",&truncation1),0);
		truncation(truncation1);
		if(DEBUG)
			cout << "FieldDescriptionGribApi - Spectral Truncation: " << truncation1 << endl;
	}
	else
	{
		// Pick up Area and Scanning Mode if the field is Grid
		// Area
		GRIB_CHECK(grib_get_double(handle,"latitudeOfFirstGridPointInDegrees",&north),0);
		GRIB_CHECK(grib_get_double(handle,"longitudeOfFirstGridPointInDegrees",&west),0);
		GRIB_CHECK(grib_get_double(handle,"latitudeOfLastGridPointInDegrees",&south),0);
		GRIB_CHECK(grib_get_double(handle,"longitudeOfLastGridPointInDegrees",&east),0);
		area(north,west,south,east);
	    // Scanning mode
	    GRIB_CHECK(grib_get_long(handle,"jScansPositively",&scanningMode1),0);
		scanningMode(scanningMode1);
		if(DEBUG){
			cout << "FieldDescriptionGribApi - Latitude Of First Grid Point: " << north << endl;
			cout << "FieldDescriptionGribApi - Longitude Of First Grid Point: " << west << endl;
			cout << "FieldDescriptionGribApi - Latitude Of Last Grid Point: " << south << endl;
			cout << "FieldDescriptionGribApi - Longitude Of Last Grid Point: " << east << endl;
			cout << "FieldDescriptionGribApi - Scanning Mode: " << scanningMode1 << endl;
		}

		if(id_.reduced_) {
			 /* get reduced grid specification */
			GRIB_CHECK(grib_get_size(handle,"pl",&rgSpecLength),0);
			if(DEBUG)
				cout << "FieldDescriptionGribApi - rgSpecLength: " << rgSpecLength << endl;
			rgSpec = new long[rgSpecLength];
			GRIB_CHECK(grib_get_long_array(handle,"pl",rgSpec,&rgSpecLength),0);
			ASSERT(rgSpecLength > 0); 
			reducedGridDefinition(rgSpec,rgSpecLength);
		}
	}
	// Gaussian grid
	if(id_.gridType_ == "gaussian") {
		// Gaussian Grid Number
		GRIB_CHECK(grib_get_long(handle,"numberOfParallelsBetweenAPoleAndTheEquator",&gaussianNumber1),0);
		gaussianNumber(gaussianNumber1);
		if(DEBUG)
			cout << "FieldDescriptionGribApi - Gaussian Number: " << gaussianNumber1 << endl;
	}
	// Lat-Lon grid
	else if(id_.gridType_ == "latlon") {
		// J Direction Increment
		GRIB_CHECK(grib_get_double(handle,"jDirectionIncrementInDegrees",&nsIncrement),0);
		if(DEBUG)
			cout << "FieldDescriptionGribApi - j Direction Increment: " << nsIncrement << endl;
		if(!id_.reduced_) {
			// I Direction Increment
			GRIB_CHECK(grib_get_double(handle,"iDirectionIncrementInDegrees",&weIncrement),0);
			if(DEBUG)
				cout << "FieldDescriptionGribApi -- i Direction Increment = " << weIncrement << endl;
		}
		increments(nsIncrement,weIncrement);
	}
}

void FieldDescriptionGribApi::extractListOfPoints(grib_handle* inHandle, vector<Point>& points, const FieldDescription& output, vector<double>& outValues) const
{
    ASSERT(inHandle);

    GribApiInput  in;

    size_t valuesLength                = 0;
    GRIB_CHECK(grib_get_size(inHandle,"values",&valuesLength),0);

    // CACHING OPPORTUNITY:
    vector<double> values(valuesLength);

    //NB The following function FILLS the vector that we allocated
    auto_ptr<Field> inputField(in.defineField(inHandle, values));

    if(DEBUG)
        cout  << "ecregrid_list_of_points_point: -- Input Field => " << *inputField << endl;

    auto_ptr<Field> outputField ( new GridField(new ListOfPoints(points),output.ft_.scanningMode_,output.ft_.frameNumber_,output.ft_.bitmap_,output.ft_.bitmapFile_,output.ft_.bitsPerValue_,output.ft_.editionNumber_,output.ft_.missingValue_));

    if(DEBUG)
        cout  << "*** ecregrid_list_of_points_point: -- Output Field " << *outputField << endl;

    Factory factory;
    auto_ptr<Transformer> trans(factory.getTransformer(*inputField,*outputField,output));
    auto_ptr<Field> end (trans->transform(*inputField, *outputField));
    const GridField&    endGridField = dynamic_cast<const GridField&>(*end);

	char *dump = getenv("ECREGRID_DUMP_TO_FILE");
	if(dump!=NULL)
	    endGridField.dump2file(dump);

    endGridField.grid().generateGrid1D(points);

    ASSERT(points.size() == endGridField.dataLength());

    outValues = end->data();
}

FieldDescriptionGribApi::~FieldDescriptionGribApi()
{
}

