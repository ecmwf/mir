/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef Area_H
#define Area_H

#ifndef   machine_H
#include "machine.h"
#endif


// Forward declarations
class Point;

// class ostream;

// 

class Area {
public:

// -- Exceptions
	// None

// -- Contructors
 
//	Area();
	Area(double north,double west,double south,double east,int scMode);
	Area(double north,double west,double south,double east);
	Area(const Area& other);

// -- Destructor

	~Area(); // Change to virtual if base class

// -- Convertors
	// None

// -- Operators
	bool operator==(const Area& other) const;
	Area& operator=(const Area& other);

// -- Methods
	bool isCellCentered(double ns, double we) const;
// ----  Accessors
	double north() const;
	double west()  const;
	double east()  const;
	double south() const;

	bool northernHemisphereOnly()    const;
	bool southernHemisphereOnly()    const;
	bool symmetricalAboutEquator()   const;
	bool moreNorthThanSouth()        const;
	bool moreSouthThanNorth()        const;

	void reOrderBoundaries(int scMode);
    void check();
	bool empty() const;

	Area join(const Area& other) const;
	Area intersection(const Area& other) const;

	bool intersect(const Area& other) const;
	bool contain(const Area& other) const;
	bool contain(const Area& other,bool globalWestEast, bool globalNorthSouth) const;
	bool issame(const Area& other) const;
	bool withinWestEast(const Area& other) const;
	bool withinNorthSouth(const Area& other) const;

	bool isPointWithinArea(const Point& p) const;
	bool isGlobalWestEast(double step) const;
	bool isOnePoint() const; 

	double size() const;

// -- Overridden methods
	// None

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


// -- Members
	double north_;
	double west_;
	double south_;
	double east_;

// -- Methods
	// None

// -- Overridden methods
	// None

// -- Class members
	// None

// -- Class methods
	// None

// -- Friends

	friend ostream& operator<<(ostream& s,const Area& p)
		{ p.print(s); return s; }

};

#endif
