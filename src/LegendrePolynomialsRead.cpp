/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "LegendrePolynomialsRead.h"

#include "Exception.h"
#include "Grid.h"

#include <unistd.h>
#include <sys/stat.h>
#include <sstream>
#include <fcntl.h>

LegendrePolynomialsRead::LegendrePolynomialsRead(int truncation, const Grid& grid):
	LegendrePolynomials(truncation), gridInfo_(grid.coeffInfo()), latLength_((truncation + 1) * (truncation + 4) / 2)
{
    latSize_ = latLength_ * sizeof(double);

    path_ = getDataDir();
    path_.append("/legendre_polynomials");

    checkAndPossiblyCreate(grid);
}

LegendrePolynomialsRead::~LegendrePolynomialsRead()
{
}

void LegendrePolynomialsRead::checkAndPossiblyCreate(const Grid& grid)
{
	string fileName = constructCoefficientsFilename();
	if(DEBUG)
		cout <<"LegendrePolynomialsRead::checkAndPossiblyCreate filename " << fileName << endl;
	FILE* f = ::fopen64(fileName.c_str(), "r");
	if (f) {
		auto_ptr<Grid>globalGrid( grid.getGlobalGrid() );
		vector<double> globalLatitudes; 
		globalGrid->latitudes(globalLatitudes);
		int nshalf = (1+globalGrid->northSouthNumberOfPoints())/2;
		if(!checkFileSize(f, nshalf))
        {
            stringstream err;
            err << "File " << fileName << " is not of expected length";
            throw CantOpenFile("LegendrePolynomialsRead::checkAndPossiblyCreate " + err.str());
        }
		
        fclose(f);

		return;
	}
	else
		createWholeGlobeAndWriteToFile(grid);
}

void LegendrePolynomialsRead::createWholeGlobeAndWriteToFile(const Grid& grid) const
{
	string fileNameTemp = constructFilename("/xecregrid_cf_t");
	string fileName = constructCoefficientsFilename();

	FILE* file = ::fopen64(fileNameTemp.c_str(), "wa");
	if (!file)
		throw CantOpenFile("LegendrePolynomialsRead::createWholeGlobeAndWriteToFile " + fileName);

	if(chmod( fileNameTemp.c_str(), (mode_t) 0444 ))
		throw CantOpenFile("LegendrePolynomialsRead::createWholeGlobeAndWriteToFile cannot change mode to 444" + fileName);
		

	auto_ptr<Grid>globalGrid( grid.getGlobalGrid() );
	vector<double> lats;
	globalGrid->northHemisphereLatitudes(lats);
	int latsNumber = lats.size();

	size_t sizeC = latLength_ * sizeof(double);
	double* legendre = new double[latLength_]; ASSERT(legendre);
	int     ilim     = truncation_ + 1;
	double* work     = new double[3*(ilim)]; ASSERT(work);

	for ( int j = 0 ; j < latsNumber  ; j++ ) {
		calculateLegendrePoly(legendre,work,lats[j]);
		ASSERT(::fwrite((char*)legendre,1,sizeC,file) == sizeC);
	}

	delete [] work;
	delete [] legendre;
	ASSERT(::fclose(file) == 0);
// move to right name	
	if(::rename( fileNameTemp.c_str(), fileName.c_str() ))
		throw CantOpenFile("LegendrePolynomials::createWholeGlobeAndWriteToFile cannot move to right name " + fileName);
}

bool LegendrePolynomialsRead::checkFileSize(FILE* f, int globalLatNumber)
{
	if (f)
		fseek(f,0,2);
	else
	       	throw CantOpenFile("LegendrePolynomials::checkFileSize " + gridInfo_);

	if((latLength_ * globalLatNumber * sizeof(double)) == ftell(f))
		return true;

	return false;
}



void LegendrePolynomialsRead::print(ostream& out) const
{
	out << "Legendre Polynomials file name: " << constructCoefficientsFilename() << endl;
	LegendrePolynomials::print(out);
}
    
string LegendrePolynomialsRead::constructFilename(const string& stub) const
{
    stringstream fileName;
    fileName << path_ << stub << truncation_ << "_" <<  gridInfo_;
    return fileName.str();
}

string LegendrePolynomialsRead::constructCoefficientsFilename() const
{
    return constructFilename("/ecregrid_cf_t");
}

bool LegendrePolynomialsRead::openCoefficientsFile(int flags/*=0*/) const
{
    string fileName = constructCoefficientsFilename();
    fd_ = open64(fileName.c_str(),O_RDONLY|flags);

    if (fd_ < 0)
        throw CantOpenFile("LegendrePolynomialsRead::openCoefficientsFile " + fileName);

    return fd_ >= 0;

}

void LegendrePolynomialsRead::closeFile() const
{
    if(fd_ >= 0)
        close(fd_);
}
