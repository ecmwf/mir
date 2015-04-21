/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "NearestNeigbourAdjusted.h"

#ifndef Exception_H
#include "Exception.h"
#endif

#ifndef Point_H
#include "Point.h"
#endif

#ifndef FieldPoint_H
#include "FieldPoint.h"
#endif

NearestNeigbourAdjusted::NearestNeigbourAdjusted() :
    Interpolator(4) {
}

NearestNeigbourAdjusted::NearestNeigbourAdjusted(int n) :
    Interpolator(n) {
}


NearestNeigbourAdjusted::~NearestNeigbourAdjusted() {
//cout << "NearestNeigbourAdjusted: cleaning up." << endl;
}

double NearestNeigbourAdjusted::interpolatedValue(const Point& where, const vector<FieldPoint>& nearests) const {
    int size = nearests.size();

    ASSERT( size == 4);
    /*
    	Numbering of the points (I is the interpolation point):

                    0       1
    				    I
                    2       3
    */

    /* pdlat   - Meridian linear weight */
//		double pdlat1 = (where.latitude() - nearests[0].latitude()) / (nearests[2].latitude() - nearests[0].latitude());
    double pdlat1 = (nearests[0].latitude() - where.latitude()) / (nearests[0].latitude() - nearests[2].latitude());
    double pdlat2 = 1 - pdlat1;

    // North
    double	step = fabs(nearests[1].longitude() - nearests[0].longitude());

    double lonOut = where.longitude();
//    	double distNW = (where.longitude() - nearests[0].iIndex() * step ) / step;
    double distNW =  lonOut / step + 1;
    int    dnw = int(distNW);
    distNW = distNW - dnw;
    double distNE = 1 - distNW;

    // south
    step = fabs(nearests[3].longitude() - nearests[2].longitude());
//		double distSW = (where.longitude() - nearests[2].iIndex() * step ) / step;
    double distSW = lonOut / step + 1;
    int    dsw = int(distSW);
    distSW = distSW - dsw;
    double distSE = 1 - distSW;


    double nw = distNW * distNW  +  pdlat1 * pdlat1 ;
    double ne = distNE * distNE  +  pdlat1 * pdlat1 ;
    double sw = distSW * distSW  +  pdlat2 * pdlat2;
    double se = distSE * distSE  +  pdlat2 * pdlat2;

    if((nw < ne || same(nw,ne)) && (nw < sw || same(nw,sw)) && (nw < se || same(nw,se)))
        return nearests[0].value();
    else if ( (ne < sw || same(ne,sw)) && (ne < se || same(ne,se)))
        return nearests[1].value();
    else if (sw < se || same(sw,se))
        return nearests[2].value();

    return nearests[3].value();
}

void NearestNeigbourAdjusted::print(ostream& out) const {
    Interpolator::print(out);
    out << "NearestNeigbourAdjusted" ;
}

