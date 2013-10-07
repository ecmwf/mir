/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "BiLinear.h"

#ifndef Exception_H
#include "Exception.h"
#endif

#ifndef FieldPoint_H
#include "FieldPoint.h"
#endif

#include "NearestNeigbour.h"

BiLinear::BiLinear() :
	Interpolator(4)
{
}

BiLinear::BiLinear(bool w, bool a, double nPole, double sPole) :
	Interpolator(w,a,nPole,sPole,4)
{
}

BiLinear::~BiLinear()
{ 
//cout << "BiLinear: cleaning up." << endl; 
}

/*
	Numbering of the points (I is the interpolation point):

                0       1
				    I
                2       3
*/
void BiLinear::interpolationWeights(const Point& where, const vector<FieldPoint>& nearests, vector<double>& weights) const
{
    if (nearests.size() < neighbour_)
    {
        missingNeighbourWeights(where, nearests, weights);
        return;
    }

	/* The unnormalised weights are the sizes of the opposing rectangles. */
	double weight0 = unnormalisedWeight(where, nearests[3]);
	double weight1 = unnormalisedWeight(where, nearests[2]);
	double weight2 = unnormalisedWeight(where, nearests[1]);
	double weight3 = unnormalisedWeight(where, nearests[0]);

	double sum = weight0 + weight1 + weight2 + weight3;

    weights[0] = (weight0/sum);
    weights[1] = (weight1/sum);
    weights[2] = (weight2/sum);
    weights[3] = (weight3/sum);

}

double BiLinear::interpolatedValue(const Point& where, const vector<FieldPoint>& nearests) const
{
	if(nearests.size() < neighbour_)
		return missingNeighbours(where,nearests, nearests.size());

    std::vector<double> weights(nearests.size(), 0.0);
    BiLinear::interpolationWeights(where, nearests, weights);

    double sum = 0.0;
    for (unsigned int i = 0; i < nearests.size(); i++)
        sum  += nearests[i].value() * weights[i];

    return sum;
}

void BiLinear::print(ostream& out) const
{
	Interpolator::print(out);
	out << "BiLinear" ;
}
