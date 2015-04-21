/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "DoubleLinear.h"

#ifndef Exception_H
#include "Exception.h"
#endif

#ifndef Point_H
#include "Point.h"
#endif

#ifndef FieldPoint_H
#include "FieldPoint.h"
#endif

DoubleLinear::DoubleLinear() :
    Interpolator(4) {
}

DoubleLinear::DoubleLinear(bool w, bool a, double nPole, double sPole) :
    Interpolator(w,a,nPole,sPole,4) {
}

DoubleLinear::~DoubleLinear() {
//cout << "DoubleLinear: cleaning up." << endl;
}

void DoubleLinear::interpolationWeights(const Point& where, const vector<FieldPoint>& nearests, vector<double>& weights) const {
    int size = nearests.size();
    if(size < neighbour_) {
        cout << "DoubleLinear::interpolationWeights returning missingNeighbourweights with size = " << size << endl;
        missingNeighbourWeights(where, nearests, weights);
        return;
    }
    /*
    	Numbering of the points (I is the interpolation point):

                    0       1
    				    I
                    2       3
    */

    /* pdlat   - Meridian linear weight */
    double pdlat = (where.latitude() - nearests[0].latitude()) / (nearests[2].latitude() - nearests[0].latitude());
//	double pdlat = (nearests[0].latitude() - where.latitude()) / (nearests[0].latitude() - nearests[2].latitude());

    /* pdlo1   - Zonal linear weight for the latitude of point 0. */
    double	step = fabs(nearests[1].longitude() - nearests[0].longitude());
    double pdlo1 = (where.longitude() - nearests[0].iIndex() * step ) / step;

    /* pdlo2   - Zonal linear weight for the latitude of point 2. */
    step = fabs(nearests[3].longitude() - nearests[2].longitude());
    double pdlo2 = (where.longitude() - nearests[2].iIndex() * step ) / step;

    weights.resize(neighbour_);
    weights[0] = (1.0 - pdlo2) * (1.0 - pdlat);
    weights[1] = pdlo2         * (1.0 - pdlat);
    weights[2] = (1.0 - pdlo1) *        pdlat ;
    weights[3] = pdlo1         *        pdlat ;
    stringstream ss;
    ss.precision(15);
    for (unsigned int i = 0; i < weights.size(); i++)
        ss << " " << weights[i];

    cout.precision(10);
    cout << "DoubleLinear::interpolationWeights weights " << ss.str() << endl;
}

double DoubleLinear::interpolatedValue(const Point& where, const vector<FieldPoint>& nearests) const {
    int size = nearests.size();
    if(size < neighbour_) {
        cout << "DoubleLinear::interpolatedValue returning missingNeighbours with size = " << size << endl;
        return missingNeighbours(where,nearests,size);
    }

    std::vector<double> weights;
    DoubleLinear::interpolationWeights(where, nearests, weights);
    double val = 0.0;
    cout << "interpolatedValue = ";
    for(unsigned int i = 0; i < weights.size(); i++) {
        val += weights[i] * nearests[i].value();
        cout << "("<< weights[i] <<" * " << nearests[i].value()<<")";
    }

    cout << endl;
    cout.precision(15);
    cout << "DoubleLinear::interpolationvalue value " << val << endl;
    return val;
}

void DoubleLinear::print(ostream& out) const {
    Interpolator::print(out);
    out << "DoubleLinear" ;
}
