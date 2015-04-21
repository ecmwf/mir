/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "Average.h"

#ifndef Exception_H
#include "Exception.h"
#endif

#ifndef FieldPoint_H
#include "FieldPoint.h"
#endif

#include <algorithm>

Average::Average() :
    Interpolator(16) {
}

Average::Average(int npts) :
    Interpolator(npts) {
}

Average::Average(bool w, bool a, double nPole, double sPole) :
    Interpolator(w, a, nPole, sPole, 16) {
}

Average::~Average() {
    //cout << "Average: cleaning up." << endl;
}

/*
	Numbering of the points (I is the interpolation point):

                0       1
				    I
                2       3
*/

double Average::interpolatedValue(const Point &where, const vector<FieldPoint> &nearests) const {
    int size = nearests.size();

    /*
    	vector<double> distance(size);

        for (int j = 0; j < size; j++) {
    		distance.push_back(where.sphericalDistance( nearests[j] ));
    //		distance.push_back(where.earthDistance( nearests[j] ));
    //		distance.push_back(where.quickDistance( nearests[j] ));
    	}
    	sort(distance.begin(),distance.end());
    */

    double sum = 0;
    for (int k = 0; k < size; k++) {

        sum += nearests[k].value();
    }

    //			cout << "Average::interpolatedValue !!!!!!!!!!!!!!! VALUE " << value << endl;
    return sum / size;
}

void Average::print(ostream &out) const {
    Interpolator::print(out);
    out << "Average" ;
}
