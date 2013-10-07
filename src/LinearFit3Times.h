/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef LinearFit3Times_H
#define LinearFit3Times_H

#ifndef Interpolator_H
#include "Interpolator.h"
#endif


class LinearFit3Times : public Interpolator {
public:

// -- Contructors
	LinearFit3Times();
	LinearFit3Times(bool w, bool a, double nPole, double sPole);

// -- Destructor
	~LinearFit3Times();

protected:
	void print(ostream&) const;

private:

// -- Overridden methods
    virtual void interpolationWeights(const Point& where, const vector<FieldPoint>& nearests, vector<double>& weights) const;
	double interpolatedValue(const Point& point, const vector<FieldPoint>& nearests) const;

};

#endif
