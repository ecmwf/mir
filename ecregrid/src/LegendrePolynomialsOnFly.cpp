/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "LegendrePolynomialsOnFly.h"

LegendrePolynomialsOnFly::LegendrePolynomialsOnFly(int truncation) :
	LegendrePolynomials(truncation)
{
	long latLength = (truncation + 1) * (truncation + 4) / 2;
	polynoms_.reserve(latLength);	
	work_.reserve(latLength);	
}

LegendrePolynomialsOnFly::~LegendrePolynomialsOnFly()
{
}

const double* LegendrePolynomialsOnFly::getOneLatitude(double lat, int rowOffset) const
{
	calculateLegendrePoly(&polynoms_[0],&work_[0],lat);
	return &polynoms_[0];
}

void LegendrePolynomialsOnFly::print(ostream& out) const
{
	LegendrePolynomials::print(out);
	out << "On Fly";
//	- Calculation for each Latitude";
}
