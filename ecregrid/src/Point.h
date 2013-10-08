/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef Point_H
#define Point_H

// Headers
#ifndef machine_H
#include "machine.h"
#endif

static const double FACTOR1 = 0.017453293;
static const double FACTOR2 = 57.2957763671875;

class Point {
public:

// -- Exceptions
	// None

// -- Contructors

	explicit Point(double latitude, double longitude, int i = 0, int j = 0, long k = 0);
	explicit Point(double x, double y, double z, int i = 0, int j = 0, long k = 0);
	Point(const Point& p);

// -- Destructor

	  virtual ~Point();  // Change to virtual if base class

// -- Convertors
	// None

// -- Operators

	bool operator ==(const Point& other) const
		{ return latitude_ == other.latitude_ && longitude_ == other.longitude_; }

	bool operator <(const Point& other) const
		{ return latitude_ == other.latitude_ ? longitude_ < other.longitude_ : latitude_ < other.latitude_; }

	Point& operator=(const Point& p);

// -- Methods
	double latitude()  const  { return latitude_; }
	double longitude() const  { return longitude_; }
	double x() const;
	double y() const;
	double z() const;

	double quickDistance(const Point& other) const;
	double unnormalisedWeight(const Point& other) const;
	double euclidianDistance(const Point& other) const;
	double earthDistanceInKM(const Point& other) const;
	double earthDistance(const Point& other) const;

	double sphericalDistance(const Point& other) const;

	double angle(const Point& other) const; // Earth angle between 2 points (in radian)

	double longitudesDistance(const Point& other) const;

	int    iIndex()    const  { return iIndex_; }
	int    jIndex()    const  { return jIndex_; }
	long   k1dIndex()   const  { return k1dIndex_; }

	Point  checkup() const;

protected:

// -- Members
	double latitude_;
	double longitude_;

	int    iIndex_;
	int    jIndex_;
	long   k1dIndex_;

// -- Methods
	
	 void print(ostream&) const; // Change to virtual if base class	


private:

// -- Friends

	friend ostream& operator<<(ostream& s,const Point& p)
		{ p.print(s); return s; }
};

#endif
