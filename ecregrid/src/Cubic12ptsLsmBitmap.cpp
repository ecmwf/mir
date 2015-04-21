/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "Cubic12ptsLsmBitmap.h"

#ifndef Exception_H
#include "Exception.h"
#endif

#ifndef FieldPoint_H
#include "FieldPoint.h"
#endif

#ifndef Grid_H
#include "Grid.h"
#endif


Cubic12ptsLsmBitmap::Cubic12ptsLsmBitmap(const Grid& input, const Grid& output, const string& lsmMethod, double missingValue) :
    Cubic12ptsLsm(input,output,lsmMethod), missingValue_(missingValue) {
}

Cubic12ptsLsmBitmap::Cubic12ptsLsmBitmap(bool w, bool a, double nPole, double sPole, const Grid& input, const Grid& output, const string& lsmMethod, double missingValue) :
    Cubic12ptsLsm(w,a,nPole,sPole,input,output,lsmMethod), missingValue_(missingValue) {
}

Cubic12ptsLsmBitmap::~Cubic12ptsLsmBitmap() {
}


double Cubic12ptsLsmBitmap::interpolatedValue(const Point& where, const vector<FieldPoint>& nearests) const {
    int size = nearests.size();

    if(size < 4) {
        return missingNeighbours(where,nearests,size,missingValue_);
    }

    vector<FieldPoint>::const_iterator it = nearests.begin(), end = nearests.end();
    for( ; it != end; it++) {
        if(same(it->value(),missingValue_))
            return useNearestNeigbour(where,nearests,size,missingValue_);
    }

    return Cubic12ptsLsm::interpolatedValue(where,nearests);
}

void Cubic12ptsLsmBitmap::print(ostream& out) const {
    Interpolator::print(out);
    out << "Cubic12ptsLsmBitmap" ;
}

