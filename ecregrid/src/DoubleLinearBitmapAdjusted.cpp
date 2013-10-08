/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "DoubleLinearBitmapAdjusted.h"

#ifndef Exception_H
#include "Exception.h"
#endif

#ifndef Point_H
#include "Point.h"
#endif

#ifndef FieldPoint_H
#include "FieldPoint.h"
#endif

#ifndef NearestNeigbour_H
#include "NearestNeigbour.h"
#endif

#ifndef NearestNeigbourAdjusted_H
#include "NearestNeigbourAdjusted.h"
#endif

static const double rad = 0.017453293;
DoubleLinearBitmapAdjusted::DoubleLinearBitmapAdjusted(double missingValue) :
	Interpolator(4), missingValue_(missingValue), direction_(false)
{
}

DoubleLinearBitmapAdjusted::DoubleLinearBitmapAdjusted(bool w, bool a, double nPole, double sPole, double missingValue) :
	Interpolator(w,a,nPole,sPole,4), missingValue_(missingValue), direction_(false)
{
}


DoubleLinearBitmapAdjusted::~DoubleLinearBitmapAdjusted()
{ 
//cout << "DoubleLinearBitmapAdjusted: cleaning up." << endl; 
}
 
double DoubleLinearBitmapAdjusted::interpolatedValue(const Point& where, const vector<FieldPoint>& nearests) const
{
	int size = nearests.size();

	if(size < neighbour_)
		return missingValue_;

	if(same(nearests[0].value(),missingValue_) || same(nearests[1].value(),missingValue_) || same(nearests[2].value(),missingValue_) || same(nearests[3].value(),missingValue_) ){
		NearestNeigbourAdjusted n(size);
//		NearestNeigbour n(size);
		return n.interpolatedValue(where,nearests);
	}
/*
	Numbering of the points (I is the interpolation point):

                0       1
				    I
                2       3
*/

		/* pdlat   - Meridian linear weight */
//		double pdlat1 = (where.latitude() - nearests[0].latitude()) / (nearests[2].latitude() - nearests[0].latitude());
		double pdlat1 = (nearests[0].latitude() - where.latitude()) / (nearests[0].latitude() - nearests[2].latitude());
		double pdlat2 = 1 - pdlat1;

		// North
		double	step = fabs(nearests[1].longitude() - nearests[0].longitude());

//    	double distNW = (where.longitude() - nearests[0].iIndex() * step ) / step;
		double distNW = where.longitude() / step + 1;
		   int    dnw = int(distNW);
			   distNW = distNW - dnw;
		double distNE = 1 - distNW;

        // south
				 step = fabs(nearests[3].longitude() - nearests[2].longitude());
//		double distSW = (where.longitude() - nearests[2].iIndex() * step ) / step;
		double distSW = where.longitude() / step + 1;
		   int    dsw = int(distSW);
			   distSW = distSW - dsw;
		double distSE = 1 - distSW;

		
		if(direction_){
			double value1 = nearests[0].value();
			double value2 = nearests[1].value();
			double value3 = nearests[2].value();
			double value4 = nearests[3].value();

			double cos1 = cos(value1*rad);			
			double cos2 = cos(value2*rad);			
			double cos3 = cos(value3*rad);			
			double cos4 = cos(value4*rad);			
			double sin1 = sin(value1*rad);			
			double sin2 = sin(value2*rad);			
			double sin3 = sin(value3*rad);			
			double sin4 = sin(value4*rad);			

			double c1 = cos1 * distNE + cos2 * distNW;
			double c2 = cos3 * distSE + cos4 * distSW;
			double cc = c1 * pdlat2   + c2 * pdlat1;

			double s1 = sin1 * distNE + sin2 * distNW;
			double s2 = sin3 * distSE + sin4 * distSW;
			double ss = s1 * pdlat2   + s2 * pdlat1;

			if(ss < 0)
				return atan2(ss,cc) / rad + 360.0;

			return atan2(ss,cc) / rad;
		}


		double u1 = nearests[0].value() * distNE +  nearests[1].value() * distNW;
		double u2 = nearests[2].value() * distSE +  nearests[3].value() * distSW;

		return u1 * pdlat2 + u2 * pdlat1;
}

void DoubleLinearBitmapAdjusted::print(ostream& out) const
{
	Interpolator::print(out);
	out << "DoubleLinearBitmapAdjusted" ;
}

