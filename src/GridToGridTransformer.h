/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef GridToGridTransformer_H
#define GridToGridTransformer_H

#ifndef   Transformer_H
#include "Transformer.h"
#endif


// Forward declarations
class Field;
class GridField;
class Grid;
class Extraction;

class GridToGridTransformer : public Transformer {
public:

// -- Contructors

	GridToGridTransformer(const string& intMethod = "default", const string& lsmMethod = "10min", int npts = 16, const string& type = "interpolation", const string& extrapolate = "default", double missingValue = MISSING_VALUE);

// -- Destructor

	~GridToGridTransformer(); // Change to virtual if base class

// -- Methods
	bool   transform(const GridField& in, const Grid& outGrid, const Extraction* extraction, vector<double>& values) const;

// -- Overridden methods
	Field* transform(const Field& input, const Field& output ) const;
	Wind*  transformVector(const Field& inU, const Field& inV, const Field& req) const;

protected:

// -- Members
	// None

// -- Methods
	
// -- Class members
	// None

// -- Class methods
	// None

private:

// No copy allowed

	GridToGridTransformer(const GridToGridTransformer&);
	GridToGridTransformer& operator=(const GridToGridTransformer&);

// -- Members
	string interpolationMethod_;
	string lsmMethod_;
	int    pointsForInterpolation_;
	string type_;
	string extrapolate_;

	double missingValue_;

// -- Methods
	void meridionalPartialDerivatives(const GridField& input, vector<double>& values) const;
    void zonalPartialDerivatives(const GridField& input, vector<double>& values) const;
	// None

// -- Class members
	// None

// -- Class methods
	// None


};

#endif
