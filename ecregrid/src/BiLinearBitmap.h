/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef BiLinearBitmap_H
#define BiLinearBitmap_H

#ifndef BiLinear_H
#include "BiLinear.h"
#endif



class BiLinearBitmap : public BiLinear {
public:

// -- Contructors
	BiLinearBitmap(double missingValue);
	BiLinearBitmap(bool w, bool a, double nPole, double sPole, double missingValue);

// -- Destructor
	~BiLinearBitmap();

// -- Overridden methods
	virtual double interpolatedValue(const Point& point, const vector<FieldPoint>& nearests) const;

protected:
	void print(ostream&) const;

private:
	double missingValue_;

};

#endif
