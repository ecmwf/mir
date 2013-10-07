/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "GribApiOutput.h"

#ifndef Exception_H
#include "Exception.h"
#endif

#ifndef Field_H
#include "Field.h"
#endif

#ifndef Grid_H
#include "Grid.h"
#endif

#ifndef GridField_H
#include "GridField.h"
#endif

#ifndef Point_H
#include "Point.h"
#endif

#ifndef Input_H
#include "Input.h"
#endif

#ifndef GribApiInput_H
#include "GribApiInput.h"
#endif

GribApiOutput::GribApiOutput():
	Output()
{
}

GribApiOutput::GribApiOutput(const string& name):
	Output(name)
{
}

GribApiOutput::~GribApiOutput()
{
}

void GribApiOutput::write(FILE* out, const vector<double>& values) const
{
  throw NotImplementedFeature("GribApiOutput::write");
}

void GribApiOutput::write(const Field& f) const
{
	createGribAndWriteToFile(f);
}

void GribApiOutput::write(FILE* out, const Field& f) const
{
	createGribAndWriteToFile(out,f);
}

void GribApiOutput::setGrib(grib_handle* handle, const Field& output, const grib_values* grib_set, int grib_set_count) const
{
	ASSERT(handle);
	const GridField& out = dynamic_cast<const GridField&>(output);

	int scMode = out.scanningMode();
	if(scMode != 1){
		if(DEBUG)
			cout << "GribApiOutput::setGrib Different Scanning Mode " << scMode << endl;
		auto_ptr<GridField> newOut(out.gridFieldForOtherScanningMode());
		setGrib(handle,*newOut,grib_set,grib_set_count);
		return;
	}

//	int paramM = out.parameter().marsParam();
	int count = 0;
	grib_values  values[1024];
	for ( ; count < grib_set_count ;  count++ )
		values[count] = grib_set[count];
	
	/* Composed Name */
	string composedName = out.grid().composedName();
	if(DEBUG)
		cout << "GribApiOutput::setGrib composedName: " << composedName << endl;
	size_t size = sizeof(composedName.c_str());
	GRIB_CHECK(grib_set_string(handle,"typeOfGrid", composedName.c_str(),&size),0);


	/* Scanning Mode */
	long jScansPositively = 0, iScansNegatively = 0;
	scanningMode(out,iScansNegatively,jScansPositively);

	if(DEBUG){
		cout << "GribApiOutput::setGrib iScansNegatively: " << iScansNegatively << endl;
		cout << "GribApiOutput::setGrib jScansPositively: " << jScansPositively << endl;
	}

	GRIB_CHECK(grib_set_long(handle,"iScansNegatively", iScansNegatively),0);
	GRIB_CHECK(grib_set_long(handle,"jScansPositively", jScansPositively),0);

	/* Centre */
	GRIB_CHECK(grib_set_long(handle,"centre", out.centre()),0);

	/* Table 2 Version */
	if(getenv("ECREGRID_TABLE2")){
		GRIB_CHECK(grib_set_long(handle,"table2Version", out.table()),0);
	}

	/* Level */
	GRIB_CHECK(grib_set_long(handle,"level", out.level()),0);

	/* Units */
	//GRIB_CHECK(grib_set_string(handle,"units", out->units().c_str()),0);
	/* Level Type */
	//GRIB_CHECK(grib_set_string(handle,"levelType", out.levelType().c_str()),0);
	/* Date */
	//GRIB_CHECK(grib_set_long(handle,"dataDate", out.date()),0);
	/* Time */
	//GRIB_CHECK(grib_set_long(handle,"dataTime", out.time()),0);
	/* stepUnits */
	//GRIB_CHECK(grib_set_string(handle,"stepUnits", out.stepUnits().c_str()),0);
	/* start Step */
	//GRIB_CHECK(grib_set_long(handle,"startStep", out.startStep()),0);
	/* end Step */
	//GRIB_CHECK(grib_set_long(handle,"endStep", out.endStep()),0);


	/* Accuracy */
	GRIB_CHECK(grib_set_long(handle,"bitsPerValue", out.bitsPerValue()),0);

	/* Area */
	GRIB_CHECK(grib_set_double(handle,"latitudeOfFirstGridPointInDegrees", out.grid().north()),0);
	GRIB_CHECK(grib_set_double(handle,"longitudeOfFirstGridPointInDegrees", out.grid().west()),0);
	if(composedName != "polar_stereographic"){	
		GRIB_CHECK(grib_set_double(handle,"latitudeOfLastGridPointInDegrees", out.grid().south()),0);
		GRIB_CHECK(grib_set_double(handle,"longitudeOfLastGridPointInDegrees", out.grid().east()),0);
	}

    string gridType = out.grid().gridType();

	if (gridType == "latlon") {
		/* J Direction Increment */
		GRIB_CHECK(grib_set_double(handle,"jDirectionIncrementInDegrees", out.grid().northSouthIncrement()),0);

		if(!out.grid().reduced()) {
			/* I Direction Increment */
			GRIB_CHECK(grib_set_double(handle,"iDirectionIncrementInDegrees", out.grid().westEastIncrement()),0);
			/* Number Of Points Along A Parallel  */
			GRIB_CHECK(grib_set_long(handle,"numberOfPointsAlongAParallel", out.grid().westEastNumberOfPoints()),0);
		}
		/* Number Of Points Along A Meridian */
		GRIB_CHECK(grib_set_long(handle,"numberOfPointsAlongAMeridian", out.grid().northSouthNumberOfPoints()),0);
	}
	else if (gridType == "gaussian") {
		/* I Direction Increment */
		if(!out.grid().reduced()) {
			GRIB_CHECK(grib_set_double(handle,"iDirectionIncrementInDegrees", out.grid().westEastIncrement()),0);
			/* number Of Points Along A Parallel  */
			GRIB_CHECK(grib_set_long(handle,"numberOfPointsAlongAParallel", out.grid().westEastNumberOfPoints()),0);
		}
		/* number Of Points Along A Meridian */
		GRIB_CHECK(grib_set_long(handle,"numberOfPointsAlongAMeridian", out.grid().northSouthNumberOfPoints()),0);
		/* Gaussian Number */
		GRIB_CHECK(grib_set_long(handle,"numberOfParallelsBetweenAPoleAndTheEquator", out.grid().poleEquatorNumberOfPoints()),0);

	}
	else if (gridType == "projection") {
		if(composedName == "polar_stereographic") {
			GRIB_CHECK(grib_set_long(handle,"Nx", out.grid().westEastNumberOfPoints()),0);
			if(DEBUG)
				cout << "GribApiOutput::setGrib Nx: " << out.grid().westEastNumberOfPoints() << endl;
			GRIB_CHECK(grib_set_long(handle,"Ny", out.grid().northSouthNumberOfPoints()),0);
			GRIB_CHECK(grib_set_long(handle,"DxInMetres", out.grid().westEastIncrement()),0);
			GRIB_CHECK(grib_set_long(handle,"DyInMetres", out.grid().northSouthIncrement()),0);
			GRIB_CHECK(grib_set_long(handle,"orientationOfTheGridInDegrees",CENTRAL_PARALEL),0);
			GRIB_CHECK(grib_set_long(handle,"projectionCentreFlag",CENTRAL_MERIDIAN),0);
			
		}
	}

	if (out.grid().rotated()) {
			GRIB_CHECK(grib_set_double(handle,"latitudeOfSouthernPoleInDegrees", out.grid().latPole()),0);
			GRIB_CHECK(grib_set_double(handle,"longitudeOfSouthernPoleInDegrees", out.grid().lonPole()),0);
	}


	int err;
	if(( err = grib_set_values(handle,values,count)) != 0) {
        for(int i = 0; i < count; i++){
            if(values[i].error)
                cout << values[i].name <<  " " << grib_get_error_message(values[i].error) << endl;
		}
		throw WrongFunctionCall("GribApiOutputGrid::setGrib -> Cannot set Grib headers");
    }
	if(DEBUG)
		for(int i = 0; i < count ; i++) {
			cout << "GribApiOutputGrid::setGrib -> " << values[i].name << " = ";;
			switch(values[i].type) {
        	    case GRIB_TYPE_LONG:   cout << values[i].long_value   << endl; break;
				case GRIB_TYPE_DOUBLE: cout << values[i].double_value << endl; break;
				case GRIB_TYPE_STRING: cout << values[i].string_value << endl; break;
        	}
    	}

	if(out.grid().reduced()) {
//		size_t size;
//		const long* pointNumbers = out.grid().getGridDefinition(&size);
		vector<long> pointNumbers;
		size_t size = out.grid().getGridDefinition(pointNumbers);
		if(!size)
			throw WrongValue("GribApiOutputGrid::setGrib -> Cannot set pl size is: ",size);
			
		if(DEBUG)
			cout << "GribApiOutputGrid::setGrib ->-> pl size: " << size << endl;
		GRIB_CHECK(grib_set_long(handle,"PLPresent",1),0);
		GRIB_CHECK(grib_set_long_array(handle,"pl",&pointNumbers[0],size),0);
	}
	/* Bitmap  */
	if(out.bitmap()){
		if(DEBUG){
			cout << "GribApiOutputGrid::setGrib bitmap: " << out.bitmap() << endl;
			cout << "GribApiOutputGrid::setGrib missing Value: " << out.missingValue() << endl;
		}
		GRIB_CHECK(grib_set_double(handle,"missingValue", out.missingValue()),0);
		GRIB_CHECK(grib_set_long(handle,"bitmapPresent", out.bitmap()),0);
	}


	if(DEBUG)
		cout << "GribApiOutputGrid::setGrib -> data length = " << out.dataLength() << endl;
//	out.dump2file("./tu");

	/* Set output data array */
    const double* pData = 0;
    if (out.dataLength() > 0)
        pData = &out.data()[0];

    GRIB_CHECK(grib_set_double_array(handle,"values",pData,(size_t)out.dataLength()),0);
}

grib_handle* GribApiOutput::setGrib(const Field& output, const grib_values* grib_set, int grib_set_count) const
{
	grib_handle* h;
	if( output.editionNumber() == 1 || output.editionNumber() == 0)
        h = grib_handle_new_from_template(NULL,"GRIB1");
    else
       h = grib_handle_new_from_template(NULL,"GRIB2");

	ASSERT(h);

	if(DEBUG)
		cout << "GribApiOutput::setGrib Create grib_handle for  editionNumber: " << output.editionNumber() << endl;

	setGrib(h,output,grib_set,grib_set_count);
	return h;
}
grib_handle* GribApiOutput::createSetGribHandle(const Field& out) const
{
	grib_handle* handle = setGrib(out);
	GRIB_CHECK(grib_set_long(handle,"paramId", out.number()),0);
	return handle;
}

grib_handle* GribApiOutput::createSetCopyGribHandle(const Field& out, grib_handle* hin, const grib_values* grib_set, int grib_set_count) const
{
	grib_handle* hout = setGrib(out,grib_set,grib_set_count);
	ASSERT(hout);
	if(!hin)
		return hout;

// This is produce difference in results
	int what=GRIB_SECTION_PRODUCT | GRIB_SECTION_LOCAL;
	int err = 0;
	grib_handle* hend = grib_util_sections_copy(hin,hout,what,&err);
	GRIB_CHECK(err,0);
	if(DEBUG)
		cout << "GribApiOutput::createSetCopyGribHandle paramId: " << out.number() << endl;
	long paramId = out.number();

	if(out.wind() ){
		if(paramId == 0){
			if(getenv("ECREGRID_IGNORE_PARAM")){
				if(DEBUG)
					cout << "GribApiOutput::createSetCopyGribHandle paramId is 0 Not set" << endl;
			}
			else{
				throw UserError("GribApiOutput::createGribAndWriteToFile ", "paramId is 0 , check your Grib message, please!");
			}
		}
		else{
			GRIB_CHECK(grib_set_long(hend,"paramId", paramId),0);
		}
	}

	grib_handle_delete(hout);
	return hend;
}

void GribApiOutput::createGribAndWriteToFile(FILE* outf, const Field& out, grib_handle* inHandle) const
{

	ASSERT(inHandle);
	grib_handle* h = setGrib(out);
	ASSERT(h);
	
	if(getenv("ECREGRID_SIMPLE_COPY")) {
		GRIB_CHECK((grib_copy_namespace(inHandle,"time",h)),0);
		GRIB_CHECK((grib_copy_namespace(inHandle,"vertical",h)),0);
		if(getenv("ECREGRID_WRITE_CHUNKS")){
			writeToFileGribInChunks(outf,h);
		}
		else{
			writeToFileGrib(outf,h);
		}
	}
	else {
// This is produce difference in results
		int what=GRIB_SECTION_PRODUCT | GRIB_SECTION_LOCAL;
		int err = 0;
		grib_handle* hl =grib_util_sections_copy(inHandle,h,what,&err);
		GRIB_CHECK(err,0);
		long paramId = out.number();
		if(paramId == 0){
			if(getenv("ECREGRID_IGNORE_PARAM")){
				paramId = 1;
			}
			else{
				throw UserError("GribApiOutput::createGribAndWriteToFile ", "paramId is 0 , check your Grib message, please!");
			}
		}
		GRIB_CHECK(grib_set_long(hl,"paramId", paramId),0);
		if(DEBUG)
			cout << "GribApiOutput::createGribAndWriteToFile paramId: " << paramId << endl;
		if(getenv("ECREGRID_WRITE_CHUNKS")){
			writeToFileGribInChunks(outf,hl);
		}
		else{
			writeToFileGrib(outf,hl);
		}
		grib_handle_delete(hl);
	}
	grib_handle_delete(h);
}

void GribApiOutput::createGribAndWriteToFile(FILE* outf, const Field& out) const
{
	grib_handle* h = setGrib(out);
	writeToFileGrib(outf,h);
}

void GribApiOutput::createGribAndWriteToFile(const Field& out) const
{
	grib_handle* h = setGrib(out);
	GRIB_CHECK(grib_set_long(h,"paramId", out.number()),0);
	writeToFileGrib(h);
}

grib_handle*  GribApiOutput::setGrib(const Field& out) const
{
	grib_values  grib_set[1024];
	int grib_set_count = 0;
	return setGrib(out,grib_set,grib_set_count);
}

void  GribApiOutput::setGrib(grib_handle* handle, const Field& out) const
{
	grib_values  grib_set[1024];
	int grib_set_count = 0;
	setGrib(handle,out,grib_set,grib_set_count);
}

void GribApiOutput::writeToFileGrib(FILE* out, grib_handle* h) const
{
	const void* buffer = NULL;
	size_t size = 0;

	/* Get the coded message in a buffer */
	GRIB_CHECK(grib_get_message(h,&buffer,&size),0);

	if (DEBUG)
		cout << "GribApiOutput::writeToFileGrib -- Message size = "  << size << endl;

	/* Write the buffer in a file */
	if(fwrite(buffer,1,size,out) != size)
	{
		fclose(out);
		throw WriteError("GribApiOutput::writeToFileGrib -- write error:");
	}
}

void GribApiOutput::writeToFileGrib(grib_handle* h) const
{
 	/* Open output file */
	FILE* out = fopen(fileName_.c_str(),"a");
	if (! out) {
		throw CantOpenFile(fileName_);
	}

	writeToFileGrib(out,h);
}

void GribApiOutput::writeToFileGribInChunks(FILE* out, grib_handle* h) const
{
   /* Get the coded message in a buffer */
    const void* buffer = NULL;
    size_t size = 0;

    GRIB_CHECK(grib_get_message(h,&buffer,&size),0);
	if (DEBUG)
		cout << "GribApiOutput::writeToFileGribInChunks -- Message size = "  << size << endl;

    long chunk = 1024 * 1024;
    long left = size, currentChunk = chunk, written = 0;
    while (left > 0) {
        if (left < chunk)
            currentChunk = left;

        char *pos = ((char *)buffer) + written;
        if(fwrite((void *)pos,1,currentChunk,out) != (size_t)currentChunk)
        {
            fclose(out);
			throw WriteError("GribApiOutput::writeToFileGribInChunks -- write error:");
        }
        left -= chunk;
        written += currentChunk;
    }
	if(DEBUG){
    	cout << "GribApiOutput::writeToFileGribInChunks Written " << written << " Size " << size << endl;
	}

}

void GribApiOutput::writeToFileGribInChunks(grib_handle* h) const
{
 	/* Open output file */
	FILE* out = fopen(fileName_.c_str(),"a");
	if (! out) {
		throw CantOpenFile(fileName_);
	}
   /* Get the coded message in a buffer */
    const void* buffer = NULL;
    size_t size = 0;

    GRIB_CHECK(grib_get_message(h,&buffer,&size),0);
	if (DEBUG)
		cout << "GribApiOutput::writeToFileGribInChunks -- Message size = "  << size << endl;

    long chunk = 1024 * 1024;
    long left = size, currentChunk = chunk, written = 0;
    while (left > 0) {
        if (left < chunk)
            currentChunk = left;

        char *pos = ((char *)buffer) + written;
        if(fwrite((void *)pos,1,currentChunk,out) != (size_t)currentChunk)
        {
            fclose(out);
			throw WriteError("GribApiOutput::writeToFileGribInChunks -- write error:");
        }
		fflush(out);
        left -= chunk;
        written += currentChunk;
    }
	if(DEBUG){
    	cout << "GribApiOutput::writeToFileGribInChunks Written " << written << " Size " << size << endl;
	}

}

void GribApiOutput::writeToMultiFileGrib(grib_multi_handle *mh, grib_handle* h) const
{
 	/* Open output file */
	FILE* out = fopen(fileName_.c_str(),"a");
	if (! out) {
		throw CantOpenFile(fileName_);
	}
   // Write fields to file
    grib_multi_handle_write(mh, out);
    fclose(out);

    // Cleanup
    grib_handle_delete(h);
    grib_multi_handle_delete(mh);

}

void GribApiOutput::scanningMode(const GridField& out, long& iScansNegatively, long& jScansPositively) const
{
	int scanMode = out.scanningMode();

	if(DEBUG)
		cout << "GribApiOutput::scanningMode " << scanMode << endl;

	ASSERT(scanMode > 0);
	ASSERT(scanMode < 5);

	if(scanMode == 1){
		iScansNegatively = 0;
		jScansPositively = 0;
		return;
	}
	if(scanMode == 2){
		iScansNegatively = 0;
		jScansPositively = 1;
		return;
	}
	if(scanMode == 3){
		iScansNegatively = 1;
		jScansPositively = 0;
		return;
	}
	if(scanMode == 4){
		iScansNegatively = 1;
		jScansPositively = 1;
		return;
	}
}
