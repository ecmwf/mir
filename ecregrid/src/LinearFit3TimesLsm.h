/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef LinearFit3TimesLsm_H
#define LinearFit3TimesLsm_H

#ifndef InterpolatorLsm_H
#include "InterpolatorLsm.h"
#endif


class LinearFit3TimesLsm : public InterpolatorLsm {
public:

// -- Contructors
	LinearFit3TimesLsm(const Grid& input, const Grid& output, const string& lsmMethod);
	LinearFit3TimesLsm(bool w, bool a, double nPole, double sPole, const Grid& input, const Grid& output, const string& lsmMethod);

// -- Destructor
	~LinearFit3TimesLsm();

protected:
	void print(ostream&) const;
// -- Overridden methods
	virtual double interpolatedValue(const Point& point, const vector<FieldPoint>& nearests) const;

private:


};

#endif
