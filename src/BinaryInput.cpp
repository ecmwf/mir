/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "BinaryInput.h"

#ifndef Exception_H
#include "Exception.h"
#endif

#include <fstream>

BinaryInput::BinaryInput():
	Input(),triger_(true)
{
}

BinaryInput::BinaryInput(const string& name):
	Input(name),triger_(true)
{
}

BinaryInput::~BinaryInput()
{
}

bool BinaryInput::next(FILE* fp)
{
	if(triger_ == true){
        triger_ = false;
        return true;
    }
    return false;
}

Input* BinaryInput::newInput(const string& name) const
{
	    return new BinaryInput(name);
}

bool* BinaryInput::getLsmBoolValues(size_t* valuesLength) const
{
	throw NotImplementedFeature("BinaryInput::getLsmBoolValues");
}

Grid* BinaryInput::defineGridForCheck(const string& path) const
{
	throw NotImplementedFeature("BinaryInput::defineGridForCheck");
}

void BinaryInput::getLatLonValues(vector<Point>& points) const
{
	throw NotImplementedFeature("BinaryInput::getLatLonValues");
}

void BinaryInput::getDoubleValues(const string& name, vector<double>& values)  const
{
	fstream file;
	file.open((name+fileName_).c_str(), ios::in | ios::binary);

    if (!file.is_open())
        throw CantOpenFile(name+fileName_);

    file.seekg(0, ios::end);    // restart reading at postion +0bytes from the end
    ifstream::pos_type size = file.tellg();

	cout << "BinaryInput::getDoubleValues => Size of File " << size << endl;

    file.seekg(0, ios::beg);    // restart reading at postion +0bytes from the beginning
    values.resize(size);

	file.read((char*)&values[0], size);
	// see how many bytes have been read
	cout << file.gcount() << " bytes read " << endl;

//    if (!file.fail())
	if(file.gcount() != size)
		throw ReadError("BinaryInput::getDoubleValues " + name + fileName_);

}

long* BinaryInput::getReducedGridSpecification(size_t* valuesLength) const
{
	throw NotImplementedFeature("BinaryInput::getReducedGridSpecification");
}

void BinaryInput::read(char* buffer, size_t length) const
{
	fstream file;
	file.open(fileName_.c_str(), ios::in | ios::binary);

    if (!file.is_open())
        throw CantOpenFile(fileName_);

    file.seekg(0, ios::end);    // restart reading at postion +0bytes from the end
    ifstream::pos_type size = file.tellg();

	size_t realSize = size;
	ASSERT(realSize == length);

    file.seekg(0, ios::beg);    // restart reading at postion +0bytes from the beginning

    file.read(buffer, length);
    if (!file.fail())
		throw ReadError(fileName_);

//	file_.close();
}
