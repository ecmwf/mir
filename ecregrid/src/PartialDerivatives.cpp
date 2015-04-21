/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "PartialDerivatives.h"

#ifndef Exception_H
#include "Exception.h"
#endif

#ifndef Grid_H
#include "Grid.h"
#endif

#ifndef FieldPoint_H
#include "FieldPoint.h"
#endif

const double EARTH_RADIUS = 6371229;
inline double deg2rad(double x) {
    return x/180.0*M_PI;
}

PartialDerivatives::PartialDerivatives() {
}

PartialDerivatives::~PartialDerivatives() {
}

void PartialDerivatives::klm(const vector<Point> gridPoints, const Grid& grid, int scMode, const vector<double>& data, vector<double>& k, vector<double>& l, vector<double>& m, double missingValue) const {
    const size_t valuesSize = gridPoints.size();
    ASSERT(k.size() == valuesSize);
    ASSERT(l.size() == valuesSize);
    ASSERT(m.size() == valuesSize);
    ASSERT(data.size() == valuesSize);

    vector<FieldPoint> nearest;
    nearest.reserve(4);
    for (unsigned int i = 0 ; i < valuesSize ; i++) {
        grid.nearestsByIndex(gridPoints[i],nearest,data,scMode,missingValue,4);

        double targetLat   = deg2rad(gridPoints[i].latitude());
        double targetLon   = deg2rad(gridPoints[i].longitude());
        double targetValue = data[i];

        double mer = meridional(targetLat,targetValue,nearest,missingValue);
        double zon = zonal(targetLat,targetLon,targetValue,nearest,missingValue);

        k[i] = (zon * zon + mer * mer) * 0.5;
        l[i] = (zon * zon - mer * mer) * 0.5;
        m[i] = zon * mer;

    }
}

void PartialDerivatives::zonalDerivatives(const vector<Point> gridPoints, const Grid& grid, int scMode, const vector<double>& data, vector<double>& derivatives, double missingValue) const {
    const size_t valuesSize = gridPoints.size();
    ASSERT(derivatives.size() == valuesSize);
    ASSERT(data.size() == valuesSize);

    vector<FieldPoint> nearest;
    nearest.reserve(4);
    for (unsigned int i = 0 ; i < valuesSize ; i++) {
        grid.nearestsByIndex(gridPoints[i],nearest,data,scMode,missingValue,4);

        double targetLat   = deg2rad(gridPoints[i].latitude());
        double targetLon   = deg2rad(gridPoints[i].longitude());
        double targetValue = data[i];

        derivatives[i] = zonal(targetLat,targetLon,targetValue,nearest,missingValue);
    }
}

void PartialDerivatives::meridionalDerivatives(const vector<Point> gridPoints, const Grid& grid, int scMode, const vector<double>& data, vector<double>& derivatives, double missingValue) const {
    const size_t valuesSize = gridPoints.size();
    ASSERT(derivatives.size() == valuesSize);
    ASSERT(data.size() == valuesSize);

    vector<FieldPoint> nearest;
    nearest.reserve(4);
    for (unsigned int i = 0 ; i < valuesSize ; i++) {
        grid.nearestsByIndex(gridPoints[i],nearest,data,scMode,missingValue,4);

        double targetLat   = deg2rad(gridPoints[i].latitude());
        double targetValue = data[i];

        derivatives[i] = meridional(targetLat,targetValue,nearest,missingValue);
    }
}

double PartialDerivatives::meridional(double targetLat, double targetValue, const vector<FieldPoint>& nearests, double missingValue) const {
    /*
        Numbering of the points

                    0

                1   x   3

                    2
    */
    bool ldown = false, lup = false;
    double downValue = 0, upValue = 0;

    if(same(targetValue,missingValue))
        return missingValue;
//		throw WrongValue(" PartialDerivatives::meridional target values is missing", targetValue);

    double  upValueN = nearests[0].value();
    if(!same(upValueN,missingValue)) {
        downValue = (upValueN - targetValue)/(deg2rad(nearests[0].latitude()) - targetLat) ;
        ldown = true;
    }

    double  downValueN = nearests[2].value();
    if(!same(downValueN,missingValue)) {
        upValue =  (targetValue - downValueN)/(targetLat - deg2rad(nearests[2].latitude())) ;
        lup = true;
    }

    if(ldown && lup)
        return ((downValue + upValue) * 0.5 / EARTH_RADIUS);
    else if(ldown)
        return downValue / EARTH_RADIUS;
    else if(lup)
        return upValue / EARTH_RADIUS;

    return missingValue;

}

double PartialDerivatives::zonal(double targetLat, double targetLon, double targetValue, const vector<FieldPoint>& nearests, double missingValue) const {
    /*
        Numbering of the points

                    0

                1   x   3

                    2
    */
    bool lleft = false, lright = false;
    double leftValue = 0, rightValue = 0;

    if(same(targetValue,missingValue))
        return missingValue;
//		throw WrongValue(" PartialDerivative::zonal target values is missing", targetValue);

    double  rightValueN = nearests[3].value();
    if(!same(rightValueN,missingValue)) {
        leftValue = (rightValueN - targetValue)/(EARTH_RADIUS * cos(targetLat) * (deg2rad(nearests[3].longitude()) - targetLon)) ;
        lleft = true;
    }

    double  leftValueN = nearests[1].value();
    if(!same(leftValueN,missingValue)) {
        rightValue =  (targetValue - leftValueN)/(EARTH_RADIUS * cos(targetLat) *(targetLon - deg2rad(nearests[1].longitude()))) ;
        lright = true;
    }

    if(lleft && lright)
        return (leftValue + rightValue) * 0.5;
    else if(lleft)
        return leftValue;
    else if(lright)
        return rightValue;

    return missingValue;
}

/*
void PartialDerivatives::print(ostream&) const
{
}
*/
