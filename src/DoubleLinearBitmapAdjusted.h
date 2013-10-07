/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef DoubleLinearBitmapAdjusted_H
#define DoubleLinearBitmapAdjusted_H

#ifndef Interpolator_H
#include "Interpolator.h"
#endif


class DoubleLinearBitmapAdjusted : public Interpolator {
public:

// -- Contructors
	DoubleLinearBitmapAdjusted(double missingValue);
	DoubleLinearBitmapAdjusted(bool w, bool a, double nPole, double sPole, double missingValue);

// -- Destructor
	~DoubleLinearBitmapAdjusted();

// -- Overridden methods
	virtual double interpolatedValue(const Point& point, const vector<FieldPoint>& nearests) const;

protected:
    void print(ostream&) const;

private:
	double missingValue_;
	bool   direction_;

};

#endif
