/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "GribApiOutputSpectral.h"

#ifndef Exception_H
#include "Exception.h"
#endif

#ifndef Field_H
#include "Field.h"
#endif

#ifndef SpectralField_H
#include "SpectralField.h"
#endif

#include "grib_api.h"

GribApiOutputSpectral::GribApiOutputSpectral():
	GribApiOutput()
{
}

GribApiOutputSpectral::GribApiOutputSpectral(const string& name):
	GribApiOutput(name)
{
}

GribApiOutputSpectral::~GribApiOutputSpectral()
{
}


grib_handle* GribApiOutputSpectral::setGrib(const Field& output, const grib_values* grib_set, int grib_set_count) const
{
	grib_handle* h;
    if( output.editionNumber() == 1)
        h = grib_handle_new_from_samples(NULL,"sh_pl_grib1");
    else
        h = grib_handle_new_from_samples(NULL,"sh_pl_grib2");

	setGrib(h,output,grib_set,grib_set_count);

	return h;
}

void GribApiOutputSpectral::setGrib(grib_handle* h, const Field& output, const grib_values* grib_set, int grib_set_count) const
{

	const SpectralField& out = dynamic_cast<const SpectralField&>(output);

    // Set the required accuracy here
    GRIB_CHECK(grib_set_long(h,"bitsPerValue", output.bitsPerValue()),0);

	int count = 0;
	grib_values  values[1024];
	for ( ; count < grib_set_count ;  count++ )
		values[count] = grib_set[count];

/*
*/
    values[count].name         = "typeOfGrid";
    values[count].type         = GRIB_TYPE_STRING;
    values[count].string_value = "sh";
    count++;

/* Set representationMode because of EMOSLIB compatibility */
/*
        values[count].name       = "representationMode";
        values[count].type       = GRIB_TYPE_LONG;
        values[count].long_value = 2;
        count++;
*/


	/* Parameter */
	int paramM = output.parameter().marsParam();
    if(DEBUG)
        cout << "GribApiOutputGridSpectral::setGrib: parameter = " << paramM << endl;
    values[count].name         = "paramId";
    values[count].type         = GRIB_TYPE_LONG;
    values[count].long_value   = paramM;
    count++;

	/* Complex packing */
    values[count].name         = "packingType";
    values[count].type         = GRIB_TYPE_STRING;
    values[count].string_value = "spectral_complex";
    //values[count].string_value = "spectral_simple";
    count++;
	/*
	*/
	/* Level */
	values[count].name         = "level";
	values[count].type         = GRIB_TYPE_LONG;
	values[count].long_value   = out.level();
	count++;

	/* Level Type */
	/*
	values[count].name         = "levelType";
	values[count].type         = GRIB_TYPE_STRING;
	values[count].string_value = out->levelType().c_str();
	count++;
	*/

	/* Date */
	values[count].name         = "dataDate";
	values[count].type         = GRIB_TYPE_LONG;
	values[count].long_value   = out.date();
	count++;

	/* Time */
	values[count].name         = "dataTime";
	values[count].type         = GRIB_TYPE_LONG;
	values[count].long_value   = out.time();
	count++;

	int truncation = out.truncation();
        /* Set Truncation */
        values[count].name       = "pentagonalResolutionParameterJ";
        values[count].type       = GRIB_TYPE_LONG;
        values[count].long_value = truncation;
        count++;

        values[count].name       = "pentagonalResolutionParameterK";
        values[count].type       = GRIB_TYPE_LONG;
        values[count].long_value = truncation;
        count++;

        values[count].name       = "pentagonalResolutionParameterM";
        values[count].type       = GRIB_TYPE_LONG;
        values[count].long_value = truncation;
        count++;

        /*       complexPacking=1; */
        /* Set Truncation sub Set for complex packing */
        /*          subSetJ = 20; */
        /*          subSetK = 20; */
        /*          subSetM = 20; */
		/*
            values[count].name       = "computeLaplacianOperator";
            values[count].type       = GRIB_TYPE_LONG;
            values[count].long_value = 1;
            count++;
		int subSetJ = 0, subSetK = 0, subSetM = 0;
            values[count].name       = "JS";
            values[count].type       = GRIB_TYPE_LONG;
            values[count].long_value = subSetJ;
            count++;
            values[count].name       = "KS";
            values[count].type       = GRIB_TYPE_LONG;
            values[count].long_value = subSetK;
            count++;
            values[count].name       = "MS";
            values[count].type       = GRIB_TYPE_LONG;
            values[count].long_value = subSetM;
            count++;
			*/


	int err;
	if(( err = grib_set_values(h,values,count)) != 0) {
        for(int i = 0; i < count; i++){
            if(values[i].error)
                cout << values[i].name <<  " " << grib_get_error_message(values[i].error) << endl;
		}
		throw WrongFunctionCall("GribApiOutputSpectral::setGrib -> Cannot set Grib headers");
    }

	if(DEBUG)
			for(int i = 0; i < count ; i++) {
       		    cout << "GribApiOutputSpectral::setGrib -> " << values[i].name << " = ";;
        		switch(values[i].type) {
            		case GRIB_TYPE_LONG:   cout << values[i].long_value   << endl; break;
            		case GRIB_TYPE_DOUBLE: cout << values[i].double_value << endl; break;
            		case GRIB_TYPE_STRING: cout << values[i].string_value << endl; break;
        		}
    		}

	if(DEBUG)
		cout << "GribApiOutputSpectral::setGrib -> data length = " << out.dataLength() << endl;
//	out->dump2file("./tu");
	/* Set output data array */
    const double* pData = 0;
    const vector<double>& data = out.data();
    if (data.size() > 0)
        pData = &data[0];

    GRIB_CHECK(grib_set_double_array(h,"values",pData,(size_t)out.dataLength()),0);

}

