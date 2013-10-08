/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef LsmFromGrid_H
#define LsmFromGrid_H

#ifndef Lsm_H
#include "Lsm.h"
#endif


// Forward declarations
class Grid;
class Interpolator;
// 

class LsmFromGrid : public Lsm {
public:

// -- Contructors

	LsmFromGrid(Input* input, Grid* grid, Interpolator* method);

// -- Destructor

	~LsmFromGrid(); // Change to virtual if base class

protected:

// -- Members
	// None

// -- Methods
	
	void print(ostream&) const; // Change to virtual if base class	

private:

// No copy allowed

	LsmFromGrid(const LsmFromGrid&);
	LsmFromGrid& operator=(const LsmFromGrid&);

// -- Members
	auto_ptr<Grid> grid_;
	auto_ptr<Interpolator> method_;
	string  directoryOfPredefined_;

// -- Methods
	void   createGlobalLsmAndWriteToFileAsGrib(const Grid&, const vector<double>& data) const;
	string  directoryOfPredefined() const { return directoryOfPredefined_; }

// -- Overridden methods
	void getLsmValues(const Grid& grid, vector<bool>& values) ;
	bool  isAvailablePredefinedLsm();
	long  value(double latitude,double longitude)    const;
	double  seaPoint(double latitude,double longitude) const;
	bool  seaPointBool(double latitude,double longitude) const;

// -- Class members
	// None

// -- Class methods
	// None

// -- Friends

	friend ostream& operator<<(ostream& s,const LsmFromGrid& p)
		{ p.print(s); return s; }

};

#endif
