/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "Cubic12pts.h"

#ifndef Exception_H
#include "Exception.h"
#endif

#ifndef FieldPoint_H
#include "FieldPoint.h"
#endif

#ifndef DoubleLinear_H
#include "DoubleLinear.h"
#endif

#include <algorithm>

Cubic12pts::Cubic12pts() :
	Interpolator(12)
{
}

Cubic12pts::Cubic12pts(bool w, bool a, double nPole, double sPole) :
	Interpolator(w,a,nPole,sPole,12)
{
}

Cubic12pts::~Cubic12pts()
{
}


inline double f2(double alpha) { return ((alpha + 1.0) * (alpha - 2.0) * (alpha - 1.0)) / 2.0; }
inline double f3(double alpha) { return - ((alpha + 1.0) * (alpha - 2.0) * alpha) / 2.0; }
inline double f4(double alpha) { return (alpha * (alpha - 1.0) * (alpha + 1.0))/6.0; }

void Cubic12pts::interpolationWeights(const Point& where, const vector<FieldPoint>& nearests, vector<double>& weights) const
{
	int size = nearests.size();
	double targetLat = where.latitude();
	double targetLon = where.longitude();

    weights.resize(nearests.size());
    weights.assign(nearests.size(), 0.0);

	if(size < 4){
        cout << "Cubic12pts::weights size < 4 so returning missingNeighbour" << endl;
		missingNeighbourWeights(where,nearests,weights);
        return;
	}
	else if(size < 12){
		DoubleLinear bilinear;
        cout << "Cubic12pts::weights size < 12 so returning missingNeighbour" << endl;
		bilinear.interpolationWeights(where,nearests, weights);	
        return;
	}

    cout << "Cubic12pts::weights size = " << size << endl;
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


//     PDLAT   - Meridian linear weight
	double pdlat = (targetLat - lat1) / (lat2 - lat1);
//     PDLO0   - Zonal linear weight for the latitude of point 4.
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

//	cout << " pdlat " << pdlat << "  zdy " << zdy << endl;

//    Polynomial in x-direction.

	// Northern parallel
	double zwxn1 = f2(pdlo1);
	double zwxn2 = f3(pdlo1);
	double zwxn3 = f4(pdlo1);
	double zwxn0 = 1.0 - zwxn1 - zwxn2 - zwxn3;

	// Southern parallel
	double zwxs1 = f2(pdlo2);
	double zwxs2 = f3(pdlo2);
	double zwxs3 = f4(pdlo2);
	double zwxs0 = 1.0 - zwxs1 - zwxs2 - zwxs3;

//    Polynomial in y-direction.

	double zwy3 = ((zdy+zdy10) * (zdy) * (zdy-zdy21)) /
                  ((zdy10+zdy21+zdy32) * (zdy21+zdy32) * (zdy32));
	double zwy2 = ((zdy+zdy10) * (zdy) * (zdy-zdy21-zdy32)) /
                  ((zdy10+zdy21) * (zdy21) * (-zdy32));
	double zwy1 = ((zdy+zdy10) * (zdy-zdy21) * (zdy-zdy21-zdy32)) /
                  ((zdy10) * (-zdy21) * (-zdy21-zdy32));
	double zwy0 = 1.0 - zwy1 - zwy2 - zwy3;

//   Linear parts for extreme rows.

	// Northernmost
	double zwxnn = pdlo0;
	double zcxnn = 1.0 - pdlo0;

	// Southernmost
	double zwxss = pdlo3;
	double zcxss = 1.0 - pdlo3;
/*
		if(where.k1dIndex() == 974347) {
			cout << endl << "out " << where.k1dIndex() << endl;
			cout << "1 " << nearests[0].k1dIndex() <<  endl;
			cout << "2 " << nearests[1].k1dIndex() <<  endl;
			cout << "3 " << nearests[2].k1dIndex() <<  endl;
			cout << "4 " << nearests[3].k1dIndex() <<  endl;
			cout << "5 " << nearests[4].k1dIndex() <<  endl;
			cout << "6 " << nearests[5].k1dIndex() <<  endl;
			cout << "7 " << nearests[6].k1dIndex() <<  endl;
			cout << "8 " << nearests[7].k1dIndex() <<  endl;
			cout << "9 " << nearests[8].k1dIndex() <<  endl;
			cout << "10 " << nearests[9].k1dIndex() <<  endl;
			cout << "11 " << nearests[10].k1dIndex() <<  endl;
			cout << "12 " << nearests[11].k1dIndex() <<  endl;
		}
*/
    weights[0] = zwxn1 * zwy1;
    weights[1] = zwxn2 * zwy1;
    weights[2] = zwxs1 * zwy2;
    weights[3] = zwxs2 * zwy2;
    weights[4] = zcxnn * zwy0;
    weights[5] = zwxnn * zwy0;
    weights[6] = zwxn0 * zwy1;
    weights[7] = zwxn3 * zwy1;
    weights[8] = zwxs0 * zwy2;
    weights[9] = zwxs3 * zwy2;
    weights[10]= zcxss * zwy3;
    weights[11]= zwxss * zwy3;
}

double Cubic12pts::interpolatedValue(const Point& where, const vector<FieldPoint>& nearests) const
{
//	cout << "Cubic12pts::interpolatedValue  " << nearests.size() << endl;

	int size = nearests.size();
	double targetLat = where.latitude();
	double targetLon = where.longitude();

	if(size < 4){
        cout << "Cubic12pts::value size < 4 so returning missingNeighbour" << endl;
		return missingNeighbours(where,nearests,size);
	}
	else if(size < 12){
        cout << "Cubic12pts::value size < 12 so returning missingNeighbour" << endl;
		DoubleLinear bilinear;
		return bilinear.interpolatedValue(where,nearests);	
	}

    cout << "Cubic12pts::value size = " << size << endl;
    std::vector<double> weights;
    Cubic12pts::interpolationWeights(where, nearests, weights);
    double val = 0.0;
    for (unsigned int i = 0; i < 12; i++)
        val += nearests[i].value() * weights[i];
    cout << "Cubic12pts::interpolatedValue" << endl;
    return val;
}

void Cubic12pts::print(ostream& out) const
{
	Interpolator::print(out);
	out << "Cubic12pts" ;
}

