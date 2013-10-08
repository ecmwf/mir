#ifndef NEARESTNEIGBOUR_H
#define NEARESTNEIGBOUR_H
/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "Interpolator.h"

class NearestNeigbour : public Interpolator {
public:

	NearestNeigbour();
	NearestNeigbour(int n);

	~NearestNeigbour();

//  Methods
	void findNearestPoints(const Grid& input, const vector<Point>& outputPoints, vector<Point>& newOutputPoints) const;
    long findNearestPointIndex(const Point& where, const vector<FieldPoint>& nearests) const;

// Overriden methods
    virtual void interpolationWeights(const Point& where, const vector<FieldPoint>& nearests, vector<double>& weights) const;
	double interpolatedValue(const Point& where, const vector<FieldPoint>& nearests) const;


protected:
	Point findNearestPoint(const Point& where, const vector<Point>& nearests) const;
	void print(ostream&) const;

private:

};
#endif
