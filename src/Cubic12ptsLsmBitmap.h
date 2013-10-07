/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef Cubic12ptsLsmBitmap_H
#define Cubic12ptsLsmBitmap_H

#ifndef Cubic12ptsLsm_H
#include "Cubic12ptsLsm.h"
#endif

// 

class Cubic12ptsLsmBitmap : public Cubic12ptsLsm {
public:

// -- Contructors

	Cubic12ptsLsmBitmap(const Grid& input, const Grid& output, const string& lsmMethod, double missingValue);
	Cubic12ptsLsmBitmap(bool w, bool a, double nPole, double sPole, const Grid& input, const Grid& output, const string& lsmMethod, double missingValue);

// -- Destructor

	~Cubic12ptsLsmBitmap(); // Change to virtual if base class

protected:
	void print(ostream&) const;

private:
// -- Overridden methods
	double interpolatedValue(const Point& where, const vector<FieldPoint>& nearests) const;

	double missingValue_;

};

#endif
