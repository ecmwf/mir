/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "NearestNeigbour.h"

#ifndef Exception_H
#include "Exception.h"
#endif

#ifndef Grid_H
#include "Grid.h"
#endif

#ifndef Point_H
#include "Point.h"
#endif

#ifndef FieldPoint_H
#include "FieldPoint.h"
#endif

#include "GridContext.h"

#include <map>
#include <algorithm>

NearestNeigbour::NearestNeigbour() :
    Interpolator(4) {
}

NearestNeigbour::NearestNeigbour(int n) :
    Interpolator(n) {
}


NearestNeigbour::~NearestNeigbour() {
//cout << "NearestNeigbour: cleaning up." << endl;
}

typedef	map<unsigned int,double> mapDistance;


void NearestNeigbour::interpolationWeights(const Point& where, const vector<FieldPoint>& nearests, vector<double>& weights) const {
    cout << "NearestNeigbour::interpolationWeights" << endl;
    weights.assign(weights.size(), 0.0);
    int index = findNearestPointIndex(where, nearests);
    if (index < weights.size())
        weights[index] = 1.0;
    else
        throw WrongValue("NearestNeigbour::interpolationWeights index out of range:", index );


}

double NearestNeigbour::interpolatedValue(const Point& where, const vector<FieldPoint>& nearests) const {
    int index = findNearestPointIndex(where, nearests);
    return nearests[index].value();
}

void NearestNeigbour::findNearestPoints(const Grid& input, const vector<Point>& outputPoints, vector<Point>& newOutputPoints) const {
    vector<Point> nearests;
    nearests.reserve(4);

    auto_ptr<GridContext> ctx(input.getGridContext());

    vector<Point>::const_iterator it = outputPoints.begin(), end = outputPoints.end();
    for( ; it != end; it++) {
        input.nearest4pts(ctx.get(),*it,nearests);
        newOutputPoints.push_back(findNearestPoint(*it,nearests));
    }
}

Point NearestNeigbour::findNearestPoint(const Point& where, const vector<Point>& nearests) const {
    std::vector<FieldPoint> pts;
    for (unsigned int i = 0; i < nearests.size(); i++)
        pts.push_back(FieldPoint(nearests[i], 0.0));
    long index = findNearestPointIndex(where, pts);
    return Point(nearests[index].latitude(), nearests[index].longitude(),nearests[index].iIndex(),nearests[index].jIndex(),nearests[index].k1dIndex());
}

long NearestNeigbour::findNearestPointIndex(const Point& where, const vector<FieldPoint>& nearests) const {
    int size = nearests.size();
    if(size == 0)
        throw WrongValue("NearestNeigbour::findNearestPoint Point out of Scope - latitude:  ", where.latitude() );

    mapDistance d;

    for (int k = 0; k < size; k++) {
        d.insert( pair<int,double>(k,where.quickDistance( nearests[k])) );
    }

    return min_element(d.begin(), d.end(), Interpolator::comparer)->first;
}

void NearestNeigbour::print(ostream& out) const {
    Interpolator::print(out);
    out << "NearestNeigbour" ;
}

