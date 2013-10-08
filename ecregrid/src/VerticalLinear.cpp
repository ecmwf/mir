/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "VerticalLinear.h"

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

VerticalLinear::VerticalLinear() :
	VerticalInterpolator()
{
}

VerticalLinear::~VerticalLinear()
{ 
}
 
//double VerticalLinear::alongParalel(const Point& where, const FieldPoint& left, const FieldPoint& right) const

void VerticalLinear::interpolate(const GridField& inputUp, const GridField& inputDown, double* values, unsigned long valuesSize) const
{
/*
	double leftW  = fabs(left.longitude() - where.longitude());
	double rightW = fabs(where.longitude() - right.longitude());
	double low   = rightW/(rightW + leftW);

	return     low * left.value() + (1-low) * right.value();
	*/
	
    /*
    const double* dataUp   = inputUp.data();
	const double* dataDown = inputDown.data();
	int pressureDown =  inputDown.level();
	*/
	/*
    DO JL=KSTART,KPROF
      IAB=KLEVB(JL,JLEVP,KSLCT)-1
      PFLDO(JL,JLEVP)=PFLDI(JL,IAB)+ &
       & (PFLDI(JL,KLEVB(JL,JLEVP,KSLCT))-PFLDI(JL,IAB))* &
       & (PRPRESC(JL,JLEVP)-PRXP(JL,IAB,KSLCT))*PRXPD(JL,IAB,KSLCT)
    ENDDO

	for (int i = 0 ; i < valuesSize ; i++) {
		data[i] = dataDown[i] + (dataUp[i] - dataDown[i]) * (outPressure_ - pressureDown) * (1 /D(pressureDown)) ;
	}
	*/

}

void VerticalLinear::print(ostream& out) const
{
//	VerticalInterpolator::print(out);
	out << "VerticalLinear" ;
}

