#ifndef GridType_H
#define GridType_H
/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/


// Headers
#include "machine.h"
#include <vector>
#include <math.h>

// Forward declarations
class Point;
class FieldPoint;
class Grid;
class Rotation;
class GridContext;


class GridType {
public:

// -- Exceptions
	// None

// -- Contructors

	GridType(bool isGlobalwe, bool isGlobalns);

// -- Destructor

	virtual ~GridType(); // Change to virtual if base class

// -- Convertors
	// None

// -- Operators
	// None

// -- Methods
	virtual bool rounding() const { return false; }

	virtual void reOrderData(const vector<double>& data_in, vector<double>& data_out, int scMode) const = 0;
	virtual void generateGrid(vector<Point>& llgrid)  const = 0;
	virtual void generateUnrotatedGrid(vector<Point>& llgrid, const Rotation& rot)  const = 0;
	virtual void generateRotatedGrid(vector<Point>& llgrid, const Rotation& rot)  const = 0;

	virtual void nearestPts(GridContext*,const Point& where, vector<FieldPoint>& nearests, const vector<double>& data, int scMode, int howMany) const = 0;

	virtual void nearestIndexed(const Point& where, vector<FieldPoint>& nearest, const vector<double>& data, int scMode, double missingValue, int howMany) const = 0;

	long findWestEastNeighbours(double wlon, int& last_i, const vector<double>& longitudes, double& west, double& east, int& w, int& e) const;

	long findNorthSouthNeighbours(double wlat, int& last_j, const vector<double>& latitudes, double& north, double& south, int& n, int& s) const;

	void  unRotatedArea(const vector<Point>& grid) const;

	virtual long getIndex(int i, int j) const = 0;


protected:

// -- Members
	bool globalWestEast_;
	bool globalNorthSouth_;

// -- Methods
	double linear(double ll, double leftL, double leftValue, double rightL, double rightValue, double missingValue) const {
			if (same(leftValue,missingValue)){	
				if(same(rightValue,missingValue))
					return missingValue;
				else
					return rightValue;
			}

			if (same(rightValue,missingValue))
					return leftValue;
				
			double l = fabs(leftL - ll);
	        double r = fabs(ll - rightL);
			double w = r / (r + l);

			return w * leftValue + (1 - w) * rightValue; 
			}

	long ifIndexFirstLon(long i, long lastLon) const{
										if( i > lastLon ){
										if(globalWestEast_)
											return 0; 
										else 
											return -1; 
										} 
										return i; }

	long ifIndexLastLon(long i, long lastLon) const { 
										if( i < 0 ){
										if(globalWestEast_)
											return lastLon; 
										else 
											return -1; 
										} 
										return i; }
	
	// void print(ostream&) const; // Change to virtual if base class	

// -- Overridden methods
	// None

// -- Class members
	// None

// -- Class methods
	// None

private:

// No copy allowed

	GridType(const GridType&);
	GridType& operator=(const GridType&);

// -- Members

// -- Methods
	// None

// -- Overridden methods
	// None

// -- Class members
	// None

// -- Class methods
	// None

// -- Friends

	//friend ostream& operator<<(ostream& s,const GridType& p)
	//	{ p.print(s); return s; }

};

#endif
