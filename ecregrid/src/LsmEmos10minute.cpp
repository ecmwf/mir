/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "LsmEmos10minute.h"

#ifndef Exception_H
#include "Exception.h"
#endif

#ifndef Input_H
#include "Input.h"
#endif

LsmEmos10minute::LsmEmos10minute(Input* input):
	Lsm( input),directoryOfPredefined_(pathForGeneratedFiles_+"/emos/")
{
}

LsmEmos10minute::~LsmEmos10minute()
{
}

bool LsmEmos10minute::isAvailablePredefinedLsm()
{
	if(DEBUG)
		cout << "LsmEmos10minute::isAvailablePredefinedLsm " << directoryOfPredefined_ << endl;

	if(input_->exist(directoryOfPredefined_))
		return true;

	if(DEBUG)
		cout << "LsmEmos10minute::isAvailablePredefinedLsm Predefined Not available" << endl;
// read from 10min.grib

	auto_ptr<Input> f ( input_->newInput("/emos/10min/10min.grib"));
	f->getDoubleValues(path_, lsm10min_);
    ASSERT(lsm10min_.size() > 0);
    
    // NB this "isAvailable" method has the side effect of
    // loading lsm values into memory. Is this correct?

	return false;
}


double LsmEmos10minute::seaPoint(double lat, double lon) const
{
/*
	     - The 10 minute land sea mask file contains 1080 lines of 2160 values
		 - Each value consists of a 0 for sea or a 1 for land.
		 - The masks are assumed to be in the centre of a 10 minute square
!!!		 (e.g. the first value is assumed to be at 0  5' East and 89  55' North).
		   10 minute land sea mask characteristics:
		         - valuesPerDegre - Number of values per degree.
		         - lengthOfLine   - The length of a full line of latitude in the file
				 - numberOfLinesOfLatitude
*/

	const int valuesPerDegree         = 6;
	const int lengthOfLine            = 360 * valuesPerDegree;
	const int numberOfLinesOfLatitude = 180 * valuesPerDegree;

	long calc = lengthOfLine * numberOfLinesOfLatitude;

	int latNumber   = int((90.0 - lat) * valuesPerDegree ) + 1;

	if(latNumber > numberOfLinesOfLatitude)
		latNumber = numberOfLinesOfLatitude;

//	ILAT = MIN ( ( (JP90 - KLATV) * JPML01) / JPMULT + 1, JPLT01)

	long offset      = (latNumber - 1) * lengthOfLine;
	offset = offset + int(lon * valuesPerDegree + 0.000001);		

	if(offset > calc)
		offset = calc;

// IOFF = (KLONG (JLON) * JPML01) / JPMULT + 1

	return lsm10min_[offset]; 
}

bool LsmEmos10minute::seaPointBool(double lat, double lon) const
{
/*
	     - The 10 minute land sea mask file contains 1080 lines of 2160 values
		 - Each value consists of a 0 for sea or a 1 for land.
		 - The masks are assumed to be in the centre of a 10 minute square
!!!		 (e.g. the first value is assumed to be at 0  5' East and 89  55' North).
		   10 minute land sea mask characteristics:
		         - valuesPerDegre - Number of values per degree.
		         - lengthOfLine   - The length of a full line of latitude in the file
				 - numberOfLinesOfLatitude
*/

	const int valuesPerDegree         = 6;
	const int lengthOfLine            = 360 * valuesPerDegree;

//before	
//	int latNumber   = int((90.0 - lat) * valuesPerDegree);

	int latNumber   = int((90.0 - lat) * valuesPerDegree + 0.5);
//	int latNumber   = min(int((90.0 - lat) * valuesPerDegree + 0.5),valuesPerDegree);
//	ILAT = MIN ( ( (JP90 - KLATV) * JPML01) / JPMULT + 1, JPLT01)

	int offset      = (latNumber - 1) * lengthOfLine;

//before
//  	offset = offset + int(lon) * valuesPerDegree;

	    offset = offset + int(lon * valuesPerDegree + 0.5);		
//	    offset = offset + int(lon * valuesPerDegree) + 1;		
// IOFF = (KLONG (JLON) * JPML01) / JPMULT + 1

/*
	if( offset >= 2332800) {
		cout << "LsmEmos10minute::seaPoint offset > 2332800  reseted to " << 2332800 -1 << endl;
		offset = 2332800 - 1;
	}
*/
		if(lsm10min_[offset] == 0)
			return false;

		return true;
}

/*
void LsmEmos10minute::print(ostream&) const
{
}
*/
