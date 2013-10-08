/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef LinearFit3TimesLsmBitmap_H
#define LinearFit3TimesLsmBitmap_H

#ifndef LinearFit3TimesLsm_H
#include "LinearFit3TimesLsm.h"
#endif


class LinearFit3TimesLsmBitmap : public LinearFit3TimesLsm {
public:

// -- Contructors
	LinearFit3TimesLsmBitmap(const Grid& input, const Grid& output, const string& lsmMethod, double missingValue);
	LinearFit3TimesLsmBitmap(bool w, bool a, double nPole, double sPole, const Grid& input, const Grid& output, const string& lsmMethod, double missingValue);

// -- Destructor
	~LinearFit3TimesLsmBitmap();

protected:
	void print(ostream&) const;

private:

// -- Overridden methods
	virtual double interpolatedValue(const Point& point, const vector<FieldPoint>& nearests) const;

	double missingValue_;

};

#endif
