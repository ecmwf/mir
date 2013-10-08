/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include <algorithm>

#include "Linear.h"

#ifndef Exception_H
#include "Exception.h"
#endif

#ifndef Point_H
#include "Point.h"
#endif

#ifndef FieldPoint_H
#include "FieldPoint.h"
#endif

Linear::Linear() :
	Interpolator(4)
{
}

Linear::Linear(bool w, bool a, double nPole, double sPole) :
	Interpolator(w,a,nPole,sPole,4)
{
}

Linear::~Linear()
{ 
}
 
double Linear::alongParalel(const Point& where, const FieldPoint& left, const FieldPoint& right) const
{
	double lon = where.longitude();
	double leftW  = fabs(lon - left.longitude());
	double rightW = fabs(right.longitude() - lon);
	double low   = rightW/(rightW + leftW);

//	cout << "Linear::alongParalel " << endl;
	return     low * left.value() + (1-low) * right.value();
}
double Linear::alongMeridian(const Point& where, const FieldPoint& up, const FieldPoint& down) const
{
	double upW  = fabs(up.latitude() - where.latitude());
	double downW = fabs(where.latitude() - down.latitude());
	double low   = downW/(downW + upW);

//	cout << "Linear::alongMeridian " << endl;

	return     low * up.value() + (1-low) * down.value();
}

void Linear::parallelWeights(const Point& where, const FieldPoint& left, const FieldPoint& right, double& left_weight, double& right_weight) const
{
	double lon = where.longitude();
	double leftW  = fabs(lon - left.longitude());
	double rightW = fabs(right.longitude() - lon);
	double low   = rightW/(rightW + leftW);

    left_weight = low;
    right_weight = (1 - low);
}

void Linear::meridianWeights(const Point& where, const FieldPoint& up, const FieldPoint& down, double& up_weight, double& down_weight) const
{
	double upW  = fabs(up.latitude() - where.latitude());
	double downW = fabs(where.latitude() - down.latitude());
	double low   = downW/(downW + upW);

    up_weight = low;
    down_weight = (1-low);
}

void Linear::interpolationWeights(const Point& where, const vector<FieldPoint>& nearests, vector<double>& weights) const
{
    const int size = nearests.size();
    weights.resize(size, 0.0);
    if (size < 2)
    {
        weights[0] = 1.0;        
    }
    else if (2 == size)
    {
        // decide whether along parallel or meridian
		if(same(nearests[0].latitude(),nearests[1].latitude()))
			parallelWeights(where,nearests[0],nearests[1], weights[0], weights[1]);
        else if(same(nearests[0].longitude(),nearests[1].longitude()))
			meridianWeights(where,nearests[0],nearests[1], weights[0], weights[1]);
    }
    else if(size == 3) {
		if(same(nearests[0].latitude(),nearests[1].latitude()))
			parallelWeights(where,nearests[0],nearests[1], weights[0], weights[1]);
        else if(same(nearests[1].latitude(),nearests[2].latitude()))
			parallelWeights(where,nearests[1],nearests[2],weights[1], weights[2]);
        else if(same(nearests[0].longitude(),nearests[1].longitude()))
			meridianWeights(where,nearests[0],nearests[1], weights[0], weights[1]);
        else if(same(nearests[1].longitude(),nearests[2].longitude()))
			meridianWeights(where,nearests[1],nearests[2], weights[1], weights[2]);
	}
    else if(size == 4) {
		if(same(nearests[0].latitude(),nearests[1].latitude()))
			parallelWeights(where,nearests[0],nearests[1], weights[0], weights[1]);
        else if(same(nearests[2].latitude(),nearests[3].latitude()))
			parallelWeights(where,nearests[2],nearests[3], weights[2], weights[3]);
        else if(same(nearests[0].longitude(),nearests[2].longitude()))
			meridianWeights(where,nearests[0],nearests[2], weights[0], weights[2]);
        else if(same(nearests[1].longitude(),nearests[3].longitude()))
			meridianWeights(where,nearests[1],nearests[3], weights[1], weights[3]);
	}
    else
    {    
        // default
        missingNeighbourWeights(where, nearests, weights);
    }
}

double Linear::interpolatedValue(const Point& where, const vector<FieldPoint>& nearests) const
{
/*
	Numbering of the points (I is the interpolation point):

                0       1
				    I
                2       3
*/
	int size = nearests.size();

	ASSERT(size > 0);

    if (size <= 4)
    {
        std::vector<double> weights;
        Linear::interpolationWeights(where, nearests, weights);
        double val = 0.0;
        for (unsigned int i = 0; i < size; i++)
            val += weights[i] * nearests[i].value();
        return val;
    }

	return missingNeighbours(where,nearests,size);
}

void Linear::print(ostream& out) const
{
	Interpolator::print(out);
	out << "Linear" ;
}

