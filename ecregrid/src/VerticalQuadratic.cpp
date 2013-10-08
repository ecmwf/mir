/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "VerticalQuadratic.h"

#ifndef Exception_H
#include "Exception.h"
#endif

#ifndef Point_H
#include "Point.h"
#endif

#ifndef GridField_H
#include "GridField.h"
#endif

#include <algorithm>

VerticalQuadratic::VerticalQuadratic() :
	VerticalInterpolator()
{
}

VerticalQuadratic::~VerticalQuadratic()
{ 
}
 
//double VerticalQuadratic::alongParalel(const Point& where, const FieldPoint& left, const FieldPoint& right) const

void VerticalQuadratic::interpolate(const GridField& inputUp, const GridField& inputDown, double* values, unsigned long valuesSize) const
{
/*
	double leftW  = fabs(left.longitude() - where.longitude());
	double rightW = fabs(where.longitude() - right.longitude());
	double low   = rightW/(rightW + leftW);

	return     low * left.value() + (1-low) * right.value();
*/

}

void VerticalQuadratic::print(ostream& out) const
{
//	VerticalInterpolator::print(out);
	out << "VerticalQuadratic" ;
}
