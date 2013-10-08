/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "SpectralField.h"

#ifndef Exception_H
#include "Exception.h"
#endif

#ifndef Parameter_H
#include "Parameter.h"
#endif

#ifndef FieldDescription_H
#include "FieldDescription.h"
#endif

#include "DefinitionsCache.h"
#include <fstream>

SpectralField::SpectralField(int truncation) : 
	Field(),
    truncation_(truncation),
    data_(vector<double>())
{
}

// Input
SpectralField::SpectralField(int truncation, const Parameter& param, const string& units, int editionNumber, int centre, const string& levType, int level,int date, int time, const string& stepUnits, int startStep, int endStep, int bitsPerValue, const vector<double>& values) : 
	Field(param,units,editionNumber,centre,levType,level,date,time,stepUnits,startStep,endStep,bitsPerValue),
    truncation_(truncation),
    data_(values)
{
}

// Output after Transformation
SpectralField::SpectralField(int truncation, const Field& f, const vector<double>& data) : 
	Field(f),
    truncation_(truncation),
    data_(data)
{
}

SpectralField::SpectralField(int truncation, const Parameter& param, const Field& f, const vector<double>& data) : 
	Field(param,f),
    truncation_(truncation),
    data_(data)
{
}


void SpectralField::outputArea(double* area) const
{
	ASSERT(area);
	area[0] = 0;
	area[1] = 0;
	area[2] = 0;
	area[3] = 0;
}

int SpectralField::matchGaussian() const
{
    const string resol = getShareDir() + "/definitions/resolutions";
    ref_counted_ptr< const vector<string> > resolSpec = DefinitionsCache::get(resol);

    vector<string>::const_iterator it = resolSpec->begin();

	int truncationOut, gaussian;
	double llmin, llmax;

    while (it != resolSpec->end())
    {
        istringstream in(*it);
	    in >> truncationOut >> llmin >> llmax >> gaussian;
		if(truncationOut == truncation_){
		    return gaussian;
		}

        it++;
	}

	throw WrongGaussianNumber(gaussian);
	return -1;
}

double SpectralField::matchLatLon() const
{
	const string resol = getShareDir() + "/definitions/spectral_latlon";
    ref_counted_ptr< const vector<string> > resolSpec = DefinitionsCache::get(resol);

    vector<string>::const_iterator it = resolSpec->begin();

	int truncationOut;
	double ll = 0;

    while (it != resolSpec->end())
    {
        istringstream in(*it);
        in >> truncationOut >> ll;
		if(truncationOut == truncation_){
				return ll;
		}

        it++;
	}

	throw WrongValue("SpectralField::matchLatLon  ", truncation_);
	return -1;
}

void SpectralField::setOutputAreaAndBasics(FieldDescription& out) const
{
	Field::setOutputAreaAndBasics(out);
	out.area(0,0,0,0);
}

SpectralField::~SpectralField()
{
}

bool SpectralField::operator==(const SpectralField& input) const
{
	return truncation_ == input.truncation_ && bitsPerValue() == input.bitsPerValue();
}

FieldDescription* SpectralField::makeOutput(const FieldDescription& out) const
{
	FieldDescription* newOut = new FieldDescription;
	*newOut = out;

	if(newOut->decide())
		return newOut;

	newOut->truncation(truncation_);

	return newOut;
}

void SpectralField::resetOutput(FieldDescription* out) const
{
		out->truncation(truncation_);
}

unsigned long SpectralField::dataLengthComplex() const
{
	return dataLength() / 2;
}
void SpectralField::getDataAsComplex(vector<comp>& dataComplex ) const
{
	for(unsigned long i = 0 ; i < dataLength(); i = i + 2)
		dataComplex.push_back(comp(data_[i],data_[i+1]));		
}

void SpectralField::dump2file(const string& name) const
{
	ofstream shvalues;

//	shvalues.open(name.c_str(),ios::out | ios::app);
	shvalues.open(name.c_str(),ios::out);

	if (!shvalues)
	        throw CantOpenFile(name);

	cout << "SpectralField:dump2file " <<  name << endl; 

	for ( unsigned long i = 0 ; i <  dataLength(); i++ ) {
			shvalues << setw(7) << data_[i] << endl ;
//			shvalues << i + 1 << setw(7) << data_[i] << endl ;
	}

	shvalues.close();
}

void SpectralField::print(ostream& out) const
{
	out << "SpectralField{ " ; Field::print(out); 
	out << ", truncation=[" << truncation_ << "], dataLength=[" << dataLength()<< "] }";
}
