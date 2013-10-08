/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef RotatedRegularGaussian_H
#define RotatedRegularGaussian_H
#include "machine.h"
// Forward declarations
class Grid;
class Point;
class GridContext;
class FieldPoint;

class RotatedRegularGaussian {
public:

// -- Exceptions
	// None

// -- Contructors

	RotatedRegularGaussian();

// -- Destructor

	~RotatedRegularGaussian(); // Change to virtual if base class

// -- Overridden methods
	virtual Grid* newGrid(const Grid& other) const ;
	        Grid* getGlobalGrid()            const ;

	void nearestPoints(GridContext*,const Point& where, vector<FieldPoint>& nearests, const vector<double>& data, int scMode, int npts, const bool* lsmData) const;
	void nearestPoints(GridContext*,const Point& where, vector<FieldPoint>& nearests, const vector<double>& data, int scMode, int npts) const;

	virtual void generateGrid1D(vector<Point>&, int scMode, const bool* lsmData)    const ;
	virtual void generateGrid1D(vector<Point>&, int scMode)    const ;

	unsigned long calculatedNumberOfPoints()   const ;
	bool rotated() const { return true; }
	const long*  getGridDefinition(size_t* size) const;

	string   composedName()  const { return "rotated_gg"; }

// -- Class members
	// None

// -- Class methods
	// None

protected:

// -- Members
	// None

// -- Methods
	
	 void print(ostream&) const; // Change to virtual if base class	

// -- Overridden methods
	// None

// -- Class members
	// None

// -- Class methods
	// None

private:

// No copy allowed

	RotatedRegularGaussian(const RotatedRegularGaussian&);
	RotatedRegularGaussian& operator=(const RotatedRegularGaussian&);

// -- Members
	// None

// -- Methods
	// None

// -- Overridden methods
	// None

// -- Class members
	// None

// -- Class methods
	// None

// -- Friends

	//friend ostream& operator<<(ostream& s,const RotatedRegularGaussian& p)
	//	{ p.print(s); return s; }

};

#endif
