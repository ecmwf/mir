/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "LegendrePolynomialsReadFromFile.h"

#include "Exception.h"
#include "Grid.h"

#include <unistd.h>
#include <sstream>
#include <fcntl.h>

LegendrePolynomialsReadFromFile::LegendrePolynomialsReadFromFile(int truncation, const Grid& grid) : 
	LegendrePolynomialsRead(truncation,grid),
    filePositionPreserve_(0)
{
//	checkAndPossiblyCreate(grid);
	polynoms_.reserve(latLength_);

	openCoefficientsFile();
}

LegendrePolynomialsReadFromFile::~LegendrePolynomialsReadFromFile()
{
    closeFile();
}

const double* LegendrePolynomialsReadFromFile::getOneLatitude(double lat, int rowOffset) const
{
	       off64_t latSize = latLength_ * sizeof(double);
	       off64_t filePosition  = latSize * rowOffset;

//	if(DEBUG)
//		cout << "LegendrePolynomialsReadFromFile::getOneLatitude row: " <<  rowOffset << " latLength: " << latLength_ << " filePosition: " << filePosition << endl;

	if((off64_t)filePositionPreserve_ != filePosition){
		if(lseek64(fd_, filePosition, SEEK_SET) == -1){
    		stringstream pos;
			pos << filePosition << " in " << constructCoefficientsFilename();
			throw ReadError("LegendrePolynomialsReadFromFile::getOneLatitude  cannot seek to " + pos.str());
		}
	}
	ASSERT(lseek64(fd_, 0, SEEK_CUR) == filePosition);

	if( read( fd_, &polynoms_[0], latSize) != latSize){
	    throw ReadError("LegendrePolynomialsReadFromFile::getOneLatitude " + constructCoefficientsFilename());
	}

	filePositionPreserve_ = filePosition + latSize;

	return &polynoms_[0];
}

void LegendrePolynomialsReadFromFile::print(ostream& out) const
{
	LegendrePolynomialsRead::print(out);
	out << "Read From File";
}
