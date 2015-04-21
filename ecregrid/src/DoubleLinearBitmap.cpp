/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "DoubleLinearBitmap.h"

#ifndef Exception_H
#include "Exception.h"
#endif

#ifndef Point_H
#include "Point.h"
#endif

#ifndef FieldPoint_H
#include "FieldPoint.h"
#endif

DoubleLinearBitmap::DoubleLinearBitmap(double missingValue) :
    DoubleLinear(), missingValue_(missingValue) {
}

DoubleLinearBitmap::DoubleLinearBitmap(bool w, bool a, double nPole, double sPole, double missingValue) :
    DoubleLinear(w,a,nPole,sPole), missingValue_(missingValue) {
}

DoubleLinearBitmap::~DoubleLinearBitmap() {
//cout << "DoubleLinear: cleaning up." << endl;
}

double DoubleLinearBitmap::interpolatedValue(const Point& where, const vector<FieldPoint>& nearests) const {
    int size = nearests.size();
    if(size < neighbour_)
        return missingNeighbours(where,nearests,size,missingValue_);

    if(same(nearests[0].value(),missingValue_) || same(nearests[1].value(),missingValue_) || same(nearests[2].value(),missingValue_) || same(nearests[3].value(),missingValue_) )
        return useNearestNeigbour(where,nearests,size,missingValue_);

    return DoubleLinear::interpolatedValue(where,nearests);
}
void DoubleLinearBitmap::print(ostream& out) const {
    Interpolator::print(out);
    out << "DoubleLinearBitmap" ;
}

