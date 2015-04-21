/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "Cubic12ptsLsm.h"

#ifndef Exception_H
#include "Exception.h"
#endif

#ifndef FieldPoint_H
#include "FieldPoint.h"
#endif

#ifndef Grid_H
#include "Grid.h"
#endif

#ifndef BiLinearLsm_H
#include "BiLinearLsm.h"
#endif

#ifndef DoubleLinearLsm_H
#include "DoubleLinearLsm.h"
#endif

#include <algorithm>


Cubic12ptsLsm::Cubic12ptsLsm(const Grid& input, const Grid& output, const string& lsmMethod) :
    InterpolatorLsm(12,input,output,lsmMethod) {
}

Cubic12ptsLsm::Cubic12ptsLsm(bool w, bool a, double nPole, double sPole, const Grid& input, const Grid& output, const string& lsmMethod) :
    InterpolatorLsm(w,a,nPole,sPole,12,input,output,lsmMethod) {
}

Cubic12ptsLsm::~Cubic12ptsLsm() {
}

inline double adjust(double a, double b) {
    return 1.0 - int(b + 1.e-5) * (1.0 - a) ;
}

inline double f2l(double alpha, double beta, double gamma, double delta, double epsilon) {
    return
        alpha * ((1.0 + beta*gamma)*
                 (1.0 + delta*(gamma-2.0)) *
                 (1.0 + epsilon*(gamma-3.0))) /
        ((1.0 - 2.0*delta) * (1.0 - 3.0*epsilon));
}

inline double f3l(double alpha, double beta, double gamma, double delta, double epsilon) {
    return
        alpha * ((1.0 + beta*gamma) *
                 (1.0 + delta*(gamma - 1.0)) *
                 (1.0 + epsilon*(gamma - 3.0))) /
        ((1.0 + beta) * (1.0 - 2.0*epsilon));
}

inline double f4l(double alpha, double beta, double gamma, double delta, double epsilon) {
    return
        alpha * ((1.0 + beta*gamma) *
                 (1.0 + delta*(gamma - 1.0)) *
                 (1.0 + epsilon*(gamma - 2.0))) /
        ((1.0 + 2.0*beta) * (1.0 + delta)) ;
}


double Cubic12ptsLsm::interpolatedValue(const Point& where, const vector<FieldPoint>& nearests) const {
//	cout << "Cubic12ptsLsm::interpolatedValueWithLsm  " << nearests.size() << endl;

    int size = nearests.size();
    if(size < 4) {
//		cout << "Cubic12ptsLsm::interpolatedValue NN" << where << endl;
//		double mm =  missingNeighbours(where,nearests,size,missingValue);
//		cout << " index " << where.k1dIndex() << " value " << mm << endl;
        return missingNeighbours(where,nearests,size);
    } else if(size < 12) {
        // NB we can pass dereferenced lsm data ojects here as we hold ref_counted_ptr for
        // them as class members. hence they are guaranteed to exist while this
        // call completes
        return DoubleLinearLsm::calculateInterpolatedValue(where, nearests, *inLsmData_, *outLsmData_);
    }

// ssp should be nearest with same lsm
    double zwlsi1  = 1.0;
    double zwlsi2  = 1.0;
    double zwlsi3  = 1.0;
    double zwlsi4  = 1.0;
    double zwlsi5  = 1.0;
    double zwlsi6  = 1.0;
    double zwlsi7  = 1.0;
    double zwlsi8  = 1.0;
    double zwlsi9  = 1.0;
    double zwlsi10 = 1.0;
    double zwlsi11 = 1.0;
    double zwlsi12 = 1.0;
//
    double zwlsi13 = 0.0;
    double zwlsi14 = 0.0;
    double zwlsi15 = 0.0;
    double zwlsi16 = 0.0;
    double zwlsi17 = 0.0;

    double targetLat = where.latitude();
    double targetLon = where.longitude();

    bool target = isLand( (*outLsmData_)[where.k1dIndex()] );

    // NB we can use dereferenced lsm data ojects here as we hold ref_counted_ptr for
    // them as class members. hence they are guaranteed to exist while this
    // call completes
    const vector<double>& inLsmData = *inLsmData_;

    if(isLand( inLsmData[nearests[0].k1dIndex()]) != target)
        zwlsi1 = 0;
    if(isLand( inLsmData[nearests[1].k1dIndex()]) != target)
        zwlsi2 = 0;
    if(isLand( inLsmData[nearests[2].k1dIndex()]) != target)
        zwlsi3 = 0;
    if(isLand( inLsmData[nearests[3].k1dIndex()]) != target)
        zwlsi4 = 0;
    if(isLand( inLsmData[nearests[4].k1dIndex()]) != target)
        zwlsi5 = 0;
    if(isLand( inLsmData[nearests[5].k1dIndex()]) != target)
        zwlsi6 = 0;
    if(isLand( inLsmData[nearests[6].k1dIndex()]) != target)
        zwlsi7 = 0;
    if(isLand( inLsmData[nearests[7].k1dIndex()]) != target)
        zwlsi8 = 0;
    if(isLand( inLsmData[nearests[8].k1dIndex()]) != target)
        zwlsi9 = 0;
    if(isLand( inLsmData[nearests[9].k1dIndex()]) != target)
        zwlsi10 = 0;
    if(isLand( inLsmData[nearests[10].k1dIndex()]) != target)
        zwlsi11 = 0;
    if(isLand( inLsmData[nearests[11].k1dIndex()]) != target)
        zwlsi12 = 0;
    /*
    		long bb = where.k1dIndex();
    		if(bb == 974347) {
    			cout << endl << "out " << where.k1dIndex() << " " << isLand( outLsmData_[where.k1dIndex()] ) << " lsm " <<  outLsmData_[where.k1dIndex()] << endl;
    			cout << "1 " << nearests[0].k1dIndex() << " " << isLand( inLsmData[nearests[0].k1dIndex()]) << " lsm " << inLsmData[nearests[0].k1dIndex()] << endl;
    			cout << "2 " << nearests[1].k1dIndex() << " " << isLand( inLsmData[nearests[1].k1dIndex()]) << " lsm " << inLsmData[nearests[1].k1dIndex()] << endl;
    			cout << "3 " << nearests[2].k1dIndex() << " " << isLand( inLsmData[nearests[2].k1dIndex()]) << " lsm " << inLsmData[nearests[2].k1dIndex()] << endl;
    			cout << "4 " << nearests[3].k1dIndex() << " " << isLand( inLsmData[nearests[3].k1dIndex()]) << " lsm " << inLsmData[nearests[3].k1dIndex()] << endl;
    			cout << "5 " << nearests[4].k1dIndex() << " " << isLand( inLsmData[nearests[4].k1dIndex()]) << " lsm " << inLsmData[nearests[4].k1dIndex()] << endl;
    			cout << "6 " << nearests[5].k1dIndex() << " " << isLand( inLsmData[nearests[5].k1dIndex()]) << " lsm " << inLsmData[nearests[5].k1dIndex()] << endl;
    			cout << "7 " << nearests[6].k1dIndex() << " " << isLand( inLsmData[nearests[6].k1dIndex()]) << " lsm " << inLsmData[nearests[6].k1dIndex()] << endl;
    			cout << "8 " << nearests[7].k1dIndex() << " " << isLand( inLsmData[nearests[7].k1dIndex()]) << " lsm " << inLsmData[nearests[7].k1dIndex()] << endl;
    			cout << "9 " << nearests[8].k1dIndex() << " " << isLand( inLsmData[nearests[8].k1dIndex()]) << " lsm " << inLsmData[nearests[8].k1dIndex()] << endl;
    			cout << "10 " << nearests[9].k1dIndex() << " " << isLand( inLsmData[nearests[9].k1dIndex()]) << " lsm " << inLsmData[nearests[9].k1dIndex()] << endl;
    			cout << "11 " << nearests[10].k1dIndex() << " " << isLand( inLsmData[nearests[10].k1dIndex()]) << " lsm " << inLsmData[nearests[10].k1dIndex()] << endl;
    			cout << "12 " << nearests[11].k1dIndex() << " " << isLand( inLsmData[nearests[11].k1dIndex()]) << " lsm " << inLsmData[nearests[11].k1dIndex()] << endl;
    		}
    		*/

    zwlsi13 = min(zwlsi5+zwlsi6                  ,1.0);
    zwlsi14 = min(zwlsi7+zwlsi1+zwlsi2+zwlsi8    ,1.0);
    zwlsi15 = min(zwlsi9+zwlsi3+zwlsi4+zwlsi10   ,1.0);
    zwlsi16 = min(zwlsi11+zwlsi12                ,1.0);
    zwlsi17 = min(zwlsi13+zwlsi14+zwlsi15+zwlsi16,1.0);

    zwlsi1  = adjust(zwlsi1,zwlsi14);
    zwlsi2  = adjust(zwlsi2,zwlsi14);
    zwlsi3  = adjust(zwlsi3,zwlsi15);
    zwlsi4  = adjust(zwlsi4,zwlsi15);
    zwlsi5  = adjust(zwlsi5,zwlsi13);
    zwlsi6  = adjust(zwlsi6,zwlsi13);
    zwlsi7  = adjust(zwlsi7,zwlsi14);
    zwlsi8  = adjust(zwlsi8,zwlsi14);
    zwlsi9  = adjust(zwlsi9,zwlsi15);
    zwlsi10 = adjust(zwlsi10,zwlsi15);
    zwlsi11 = adjust(zwlsi11,zwlsi16);
    zwlsi12 = adjust(zwlsi12,zwlsi16);

    zwlsi13 = adjust(zwlsi13,zwlsi17);
    zwlsi14 = adjust(zwlsi14,zwlsi17);
    zwlsi15 = adjust(zwlsi15,zwlsi17);
    zwlsi16 = adjust(zwlsi16,zwlsi17);



//	cout << "Cubic12ptsLsm::interpolatedValue  size: " << nearests.size() << endl;

    /*
    	Numbering of the points (I is the interpolation point):

    lat0->              12       4       5      13

    lat1->               6       0       1       7
                                    (I)
    lat2->               8       2       3       9

    lat3->              14      10      11      15

    */
    double lat0 = nearests[4].latitude();
    double lat1 = nearests[0].latitude();
    double lat2 = nearests[2].latitude();
    double lat3 = nearests[10].latitude();

//     pdlat   - Meridian linear weight
    double pdlat = (targetLat - lat1) / (lat2 - lat1);
//     pdlo0   - Zonal linear weight for the latitude of point 4.
    double step = fabs(nearests[5].longitude() - nearests[4].longitude());
    double pdlo0 = (targetLon - nearests[4].iIndex() * step ) / step;

//     pdlo1   - Zonal linear weight for the latitude of point 0.
    step = fabs(nearests[1].longitude() - nearests[0].longitude());
    double pdlo1 = (targetLon - nearests[0].iIndex() * step ) / step;

//     pdlo2   - Zonal linear weight for the latitude of point 2.
    step = fabs(nearests[3].longitude() - nearests[2].longitude());
    double pdlo2 = (targetLon - nearests[2].iIndex() * step ) / step;

//     pdlo3   - Zonal linear weight for the latitude of point 10.
    step = fabs(nearests[11].longitude() - nearests[10].longitude());
    double pdlo3 = (targetLon - nearests[10].iIndex() * step ) / step;

//    Setup the weights between rows.

    double zdy   = pdlat * (lat2 - lat1);
    double zdy10 =          lat1 - lat0;
    double zdy21 =          lat2 - lat1;
    double zdy32 =          lat3 - lat2;


//    Polynomial in x-direction.

    // Northern parallel
    double zdlo1 = pdlo1;
    double zwxn1 = f2l(zwlsi1,zwlsi7,zdlo1,zwlsi2,zwlsi8);
    double zwxn2 = f3l(zwlsi2,zwlsi7,zdlo1,zwlsi1,zwlsi8);
    double zwxn3 = f4l(zwlsi8,zwlsi7,zdlo1,zwlsi1,zwlsi2);
    double zwxn0 = 1.0 - zwxn1 - zwxn2 - zwxn3;


//	cout << "Cubic12ptsLsm::interpolatedValue  zwxn1: " << zwxn1 << " zwxn2 " <<  zwxn2 << "  pdlo1 " << pdlo1 << endl;
//	cout << " ----------- " << endl;

    // Southern parallel
    double zdlo2 = pdlo2;
    double zwxs1 = f2l(zwlsi3,zwlsi9,zdlo2,zwlsi4,zwlsi10);
    double zwxs2 = f3l(zwlsi4,zwlsi9,zdlo2,zwlsi3,zwlsi10);
    double zwxs3 = f4l(zwlsi10,zwlsi9,zdlo2,zwlsi3,zwlsi4);
    double zwxs0 = 1.0 - zwxs1 - zwxs2 - zwxs3;

//    Polynomial in y-direction.
    double zwy1 = zwlsi14*((1.0 + zwlsi13*(zdy + zdy10 - 1.0)) *
                           (1.0 + zwlsi15*(zdy - zdy21 - 1.0)) *
                           (1.0 + zwlsi16*(zdy - zdy21 - zdy32 - 1.0))) /
                  ((1.0 + zwlsi13*(zdy10 - 1.0)) *
                   (1.0 + zwlsi15*( -zdy21 - 1.0)) *
                   (1.0 + zwlsi16*( -zdy21 - zdy32 - 1.0)));
    double zwy2 = zwlsi15*((1.0 + zwlsi13*(zdy + zdy10 - 1.0)) *
                           (1.0 + zwlsi14*(zdy - 1.0)) *
                           (1.0 + zwlsi16*(zdy - zdy21 - zdy32 - 1.0))) /
                  ((1.0 + zwlsi13*(zdy10 + zdy21 - 1.0)) *
                   (1.0 + zwlsi14*(zdy21 - 1.0)) *
                   (1.0 + zwlsi16*( -zdy32 - 1.0)));
    double zwy3 = zwlsi16*((1.0 + zwlsi13*(zdy + zdy10 - 1.0)) *
                           (1.0 + zwlsi14*(zdy - 1.0)) *
                           (1.0 + zwlsi15*(zdy - zdy21 - 1.0))) /
                  ((1.0 + zwlsi13*(zdy10 + zdy21 + zdy32 - 1.0)) *
                   (1.0 + zwlsi14*(zdy21 + zdy32 - 1.0)) *
                   (1.0 + zwlsi15*(zdy32 - 1.0)));
    double zwy0 = 1.0 - zwy1 - zwy2 - zwy3;


//   Linear parts for extreme rows.

    // Northernmost
    double zdlo  = pdlo0;
    double zwxnn = zwlsi6 * (1.0 + zwlsi5 * (zdlo - 1.0));
    double zcxnn = 1.0 - zwxnn;

    // Southernmost
    double zdlo3 = pdlo3;
    double zwxss = zwlsi12 * (1.0 + zwlsi11 * (zdlo3 - 1.0));
    double zcxss = 1.0 - zwxss;

    return      nearests[0].value() * zwxn1 * zwy1 +
                nearests[1].value() * zwxn2 * zwy1 +
                nearests[2].value() * zwxs1 * zwy2 +
                nearests[3].value() * zwxs2 * zwy2 +
                nearests[4].value() * zcxnn * zwy0 +
                nearests[5].value() * zwxnn * zwy0 +
                nearests[6].value() * zwxn0 * zwy1 +
                nearests[7].value() * zwxn3 * zwy1 +
                nearests[8].value() * zwxs0 * zwy2 +
                nearests[9].value() * zwxs3 * zwy2 +
                nearests[10].value() * zcxss * zwy3 +
                nearests[11].value() * zwxss * zwy3;
}

void Cubic12ptsLsm::print(ostream& out) const {
    Interpolator::print(out);
    out << "Cubic12ptsLsm" ;
}

