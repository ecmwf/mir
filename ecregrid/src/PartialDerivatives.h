/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef PartialDerivatives_H
#define PartialDerivatives_H

// Headers
#ifndef   machine_H
#include "machine.h"
#endif

// Forward declarations
class Grid;
class Point;
class FieldPoint;

class PartialDerivatives {
public:

// -- Contructors
	PartialDerivatives();

// -- Destructor
	~PartialDerivatives();

	void klm(const vector<Point> gridPoints, const Grid& grid, int scMode, const vector<double>& data, vector<double>& k, vector<double>& l, vector<double>& m, double missingValue) const;

	void zonalDerivatives(const vector<Point> gridPoints, const Grid& grid, int scMode, const vector<double>& data, vector<double>& derivatives, double missingValue) const;
	void meridionalDerivatives(const vector<Point> gridPoints, const Grid& grid, int scMode, const vector<double>& data, vector<double>& derivatives, double missingValue) const;

	double meridional(double targetLat, double targetValue, const vector<FieldPoint>& nearest, double missingValue) const;

	double zonal(double targetLat, double targetLon, double targetValue, const vector<FieldPoint>& nearest, double missingValue) const;

private:

// No copy allowed
	PartialDerivatives(const PartialDerivatives&);
	PartialDerivatives& operator=(const PartialDerivatives&);

// -- Members

};

#endif
