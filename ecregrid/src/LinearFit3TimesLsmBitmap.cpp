/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "LinearFit3TimesLsmBitmap.h"

#ifndef Exception_H
#include "Exception.h"
#endif

#ifndef FieldPoint_H
#include "FieldPoint.h"
#endif

#ifndef Grid_H
#include "Grid.h"
#endif

LinearFit3TimesLsmBitmap::LinearFit3TimesLsmBitmap(const Grid& input, const Grid& output, const string& lsmMethod, double missingValue) :
    LinearFit3TimesLsm(input,output,lsmMethod), missingValue_(missingValue) {
}

LinearFit3TimesLsmBitmap::LinearFit3TimesLsmBitmap(bool w, bool a, double nPole, double sPole, const Grid& input, const Grid& output, const string& lsmMethod, double missingValue) :
    LinearFit3TimesLsm(w,a,nPole,sPole,input,output,lsmMethod), missingValue_(missingValue) {
}

LinearFit3TimesLsmBitmap::~LinearFit3TimesLsmBitmap() {
//cout << "LinearFit3TimesLsmBitmap: cleaning up." << endl;
}

/*
	Numbering of the points (I is the interpolation point):

                0       1
				    I
                2       3
*/

double LinearFit3TimesLsmBitmap::interpolatedValue(const Point& where, const vector<FieldPoint>& nearests) const {
    int size = nearests.size();
    if(size < 4)
        return missingNeighbours(where,nearests,size);

    if(same(nearests[0].value(),missingValue_) || same(nearests[1].value(),missingValue_) || same(nearests[2].value(),missingValue_) || same(nearests[3].value(),missingValue_) )
        return useNearestNeigbour(where,nearests,size,missingValue_);

    return LinearFit3TimesLsm::interpolatedValue(where,nearests);
}


void LinearFit3TimesLsmBitmap::print(ostream& out) const {
    Interpolator::print(out);
    out << "LinearFit3TimesLsmBitmap - Lsm" ;
}
