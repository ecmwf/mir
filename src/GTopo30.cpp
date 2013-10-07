/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "GTopo30.h"
#include "GTopo30Dem.h"

#ifndef Exception_H
#include "Exception.h"
#endif

#ifndef Input_H
#include "Input.h"
#endif

#include <fstream>

GTopo30::GTopo30(Input* input):
	Lsm(input), directoryOfPredefined_(pathForGeneratedFiles_ + "/gtopo/")
{

}

GTopo30::~GTopo30()
{
	for(vector<GTopo30Dem*>::iterator j = dems_.begin(); j != dems_.end(); ++j)
		delete *j;
}

double GTopo30::seaPoint(double latitude,double longitude) const
{
		if(value(latitude,longitude) == -9999)
			return 0.0;
		return 1.0;			
}

bool GTopo30::seaPointBool(double latitude,double longitude) const
{
		if(value(latitude,longitude) == -9999)
			return false;
		return true;			
}

bool GTopo30::isAvailablePredefinedLsm()
{
	if(DEBUG)
		cout << "GTopo30::isAvailablePredefinedLsm " << directoryOfPredefined_ << endl;
	if(input_->exist(directoryOfPredefined_))
		return true;

	 string lsmPath = "/gtopo/gtopo30/";

	 string demsPath = path_ + lsmPath;

	 if(GTOPO_TEMP)
	 	demsPath = pathForGeneratedFiles_ + lsmPath;

	 string dems =  demsPath + "dems";
	 ifstream in(dems.c_str());
	 if(!in)
	 	throw CantOpenFile("GTopo30::isAvailablePredefinedLsm " + dems);


	string name;
	double minLat,maxLat,minLon,maxLon,ignore;
	
//	dems_.clear();

	while(in >> name >> minLat >> maxLat >> minLon >> maxLon >> ignore >> ignore >> ignore >> ignore)
	{
		dems_.push_back(new GTopo30Dem(demsPath,name,minLat,maxLat,minLon,maxLon));	
	}

	return false;
}

long GTopo30::value(double latitude,double longitude) const
{
	while(longitude < -180) longitude += 360;
	while(longitude >  180) longitude -= 360;

	for(vector<GTopo30Dem*>::const_iterator j = dems_.begin(); j != dems_.end(); ++j)
		if((*j)->contains(latitude,longitude))
			return (*j)->value(latitude,longitude);
	 throw OutOfArea(latitude,longitude);

}
