#ifndef Average_H
#define Average_H
/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "Interpolator.h"

class Average : public Interpolator {
public:

// -- Contructors
	Average();
	Average(int n);
	Average(bool w, bool a, double nPole, double sPole);

// -- Destructor
	~Average();

protected:
	void print(ostream&) const;

private:

// -- Overridden methods
	double interpolatedValue(const Point& point, const vector<FieldPoint>& nearests) const;
};
#endif
