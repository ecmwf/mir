/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "LinearFit3Times.h"

#ifndef Exception_H
#include "Exception.h"
#endif

#ifndef FieldPoint_H
#include "FieldPoint.h"
#endif

#include <algorithm>

LinearFit3Times::LinearFit3Times() :
    Interpolator(4) {
}

LinearFit3Times::LinearFit3Times(bool w, bool a, double nPole, double sPole) :
    Interpolator(w,a,nPole,sPole,4) {
}

LinearFit3Times::~LinearFit3Times() {
//cout << "LinearFit3Times: cleaning up." << endl;
}

/*
	Numbering of the points (I is the interpolation point):

                0       1
				    I
                2       3
*/

void LinearFit3Times::interpolationWeights(const Point& where, const vector<FieldPoint>& nearests, vector<double>& weights) const {
    /*
         The weights are formed by performing a linear fit along the
         two lines of latitude. These two partial weights are normalised
         before performing a linear fit along the line of meridian.
    */
    int size = nearests.size();
    if(size < 4) {
        missingNeighbourWeights(where,nearests,weights);
        return;
    }

    double whereLon = where.longitude();
    double whereLat = where.latitude();

    double nwLon = nearests[0].longitude();
    double nwLat = nearests[0].latitude();

    double neLon = nearests[1].longitude();

    double swLon = nearests[2].longitude();
    double swLat = nearests[2].latitude();

    double seLon = nearests[3].longitude();

//	double distNwLon = fabs(whereLon - nwLon);
//	double distNeLon = fabs(neLon - whereLon);

    double distNwLon = whereLon - nwLon;
    double distNeLon = neLon - whereLon;

//	double distSwLon = fabs(whereLon - swLon);
//	double distSeLon = fabs(seLon - whereLon);

    double distSwLon = whereLon - swLon;
    double distSeLon = seLon - whereLon;

//	double distNwLat = fabs(nwLat - whereLat);
//	double distSwLat = fabs(swLat - whereLat);
    double distNwLat = nwLat - whereLat;
    double distSwLat = whereLat - swLat;

    double znorth =  distNwLon + distNeLon;
    double zsouth =  distSwLon + distSeLon;

    double pwfactNW = distNeLon * zsouth * distSwLat;
    double pwfactNE = distNwLon * zsouth * distSwLat;

    double pwfactSW = distSeLon * znorth * distNwLat;
    double pwfactSE = distSwLon * znorth * distNwLat;
    /*
    	if(where.iIndex() == 1476 && where.jIndex() == 548){
    		cout << "pwfactNW " << pwfactNW << endl;
    		cout << "pwfactNE " << pwfactNE << endl;
    		cout << "pwfactSW " << pwfactSW << endl;
    		cout << "pwfactSE " << pwfactSE << endl;
    	}
    */

    weights.resize(nearests.size(), 0.0);
    double sum = pwfactNW + pwfactNE + pwfactSW + pwfactSE;

    weights[0] = pwfactNW/sum;
    weights[1] = pwfactNE/sum;
    weights[2] = pwfactSW/sum;
    weights[3] = pwfactSE/sum;
}

double LinearFit3Times::interpolatedValue(const Point& where, const vector<FieldPoint>& nearests) const {
    /*
         The weights are formed by performing a linear fit along the
         two lines of latitude. These two partial weights are normalised
         before performing a linear fit along the line of meridian.
    */
    int size = nearests.size();
    if(size < 4)
        return missingNeighbours(where,nearests,size);

    std::vector<double> weights;
    LinearFit3Times::interpolationWeights(where, nearests, weights);
    double val = 0.0;
    for (unsigned int i = 0; i < 4; i++)
        val += weights[i] * nearests[i].value();

    return val;
    /*
    	double whereLon = where.longitude();
    	double whereLat = where.latitude();

    	double nwLon = nearests[0].longitude();
    	double nwLat = nearests[0].latitude();

    	double neLon = nearests[1].longitude();

    	double swLon = nearests[2].longitude();
    	double swLat = nearests[2].latitude();

    	double seLon = nearests[3].longitude();

    //	double distNwLon = fabs(whereLon - nwLon);
    //	double distNeLon = fabs(neLon - whereLon);

    	double distNwLon = whereLon - nwLon;
    	double distNeLon = neLon - whereLon;

    //	double distSwLon = fabs(whereLon - swLon);
    //	double distSeLon = fabs(seLon - whereLon);

    	double distSwLon = whereLon - swLon;
    	double distSeLon = seLon - whereLon;

    //	double distNwLat = fabs(nwLat - whereLat);
    //	double distSwLat = fabs(swLat - whereLat);
    	double distNwLat = nwLat - whereLat;
    	double distSwLat = whereLat - swLat;

    	double znorth =  distNwLon + distNeLon;
    	double zsouth =  distSwLon + distSeLon;

        double pwfactNW = distNeLon * zsouth * distSwLat;
        double pwfactNE = distNwLon * zsouth * distSwLat;

        double pwfactSW = distSeLon * znorth * distNwLat;
        double pwfactSE = distSwLon * znorth * distNwLat;

    //	if(where.iIndex() == 1476 && where.jIndex() == 548){
    //		cout << "pwfactNW " << pwfactNW << endl;
    //		cout << "pwfactNE " << pwfactNE << endl;
    //		cout << "pwfactSW " << pwfactSW << endl;
    //		cout << "pwfactSE " << pwfactSE << endl;
    //	}


    	double sum = pwfactNW + pwfactNE + pwfactSW + pwfactSE;

        return nearests[0].value() * (pwfactNW/sum)
             + nearests[1].value() * (pwfactNE/sum)
             + nearests[2].value() * (pwfactSW/sum)
             + nearests[3].value() * (pwfactSE/sum);
    */
}

void LinearFit3Times::print(ostream& out) const {
    Interpolator::print(out);
    out << "LinearFit3Times" ;
}
