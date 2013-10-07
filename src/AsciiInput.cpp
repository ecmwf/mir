/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef AsciiInput_H
#include "AsciiInput.h"
#endif


#ifndef Exception_H
#include "Exception.h"
#endif

#ifndef Tokenizer_H
#include "Tokenizer.h"
#endif

#ifndef FieldDescription_H
#include "FieldDescription.h"
#endif

#ifndef Field_H
#include "Field.h"
#endif

#ifndef GridField_H
#include "GridField.h"
#endif

#ifndef ListOfPoints_H
#include "ListOfPoints.h"
#endif

#ifndef Timer_H
#include "Timer.h"
#endif

#include <fstream>

AsciiInput::AsciiInput():
	Input(),triger_(true)
{
}

AsciiInput::AsciiInput(const string& name):
	Input(name),triger_(true)
{
}

AsciiInput::~AsciiInput()
{
}

bool AsciiInput::next(FILE* fp)
{
//	cout << "AsciiInput::next ................ "<< triger_ << endl;
	if(triger_ == true){
		triger_ = false;
		return true;	
	}
	return false;
}

Input* AsciiInput::newInput(const string& name) const
{
	    return new AsciiInput(name);
}

Grid* AsciiInput::defineGridForCheck(const string& path) const
{
	vector<Point> points;
	return new ListOfPoints(points);

//	throw NotImplementedFeature("AsciiInput::defineGridForCheck");
}

bool* AsciiInput::getLsmBoolValues(size_t* valuesLength) const
{
	throw NotImplementedFeature("AsciiInput::getLsmBoolValues");
}

Field* AsciiInput::defineField(const FieldDescription& dc) const
{
	if(dc.id_.gridType_ == "list") {
		vector<Point> points;
        vector<double> values;
        getLatLonAndValues(points, values);

		return new GridField(new ListOfPoints(points),dc.ft_.parameter_,dc.ft_.units_,dc.ft_.editionNumber_,dc.ft_.centre_,string(dc.ft_.levelType_),dc.ft_.level_,dc.ft_.date_,dc.ft_.time_,dc.ft_.stepUnits_,dc.ft_.startStep_,dc.ft_.endStep_,dc.ft_.bitsPerValue_,dc.ft_.scanningMode_,dc.ft_.bitmap_,values,dc.ft_.missingValue_);
	}
	else
		return Input::defineField(dc);
}

void AsciiInput::getLatLonAndValues(vector<Point>& points, vector<double>& values)      const
{
	ifstream in1(fileName_.c_str());
	if(!in1) throw CantOpenFile("AsciiInput::getLatLonAndValues " + fileName_);

	string buffer;

	// Count lines
	size_t valuesLength = 0;
	while( getline(in1,buffer) )
		valuesLength++;

	ifstream in(fileName_.c_str());
	if(!in) throw CantOpenFile("AsciiInput::getLatLonAndValues " + fileName_);

	points.reserve(valuesLength);
    values.resize(valuesLength);

	double lat, lon;

	size_t i = 0;
	while(in >> lat >> lon >> values[i++])
	{
		ASSERT(i<=valuesLength);
	    points.push_back(Point(lat,lon));
	}
}

void AsciiInput::getLatLonValues(vector<Point>& points) const
{
	Tokenizer tokens(" \t");
	ifstream in(fileName_.c_str());
    if(!in)
		throw CantOpenFile("AsciiInput::getLatLonValues " + fileName_);

	string buffer;
	while( getline(in,buffer) ) {
		vector<string> v;
		tokens(buffer,v);

		int size = v.size();
		if (size >= 2) {
			for ( int j = 0; j < size ;  j = j + 2 ) {
				double lon = atof(v[j+1].c_str());
				if(lon < 0)
					lon = lon + 360.0;
				Point point(atof(v[j].c_str()),lon);
				points.push_back(point);
			}
		}
	}
}

void AsciiInput::getDoubleValues(const string& name, vector<double>& values)  const
{
	Tokenizer tokens(" ");
	fstream in((name+fileName_).c_str(), ios::in);
	    if(!in)
			throw CantOpenFile("AsciiInput::getLatLonValues " + name+fileName_);

	string buffer;
	while( getline(in,buffer) ) {
		vector<string> v;
		tokens(buffer,v);
		for (size_t j = 0; j < v.size() ;  j++ )
		    values.push_back(atof(v[j].c_str()));
	}
}

long* AsciiInput::getReducedGridSpecification(size_t* valuesLength) const
{
	throw NotImplementedFeature("AsciiInput::getReducedGridSpecification");
}

