
// File TriangularGrid.h
// Baudouin Raoult - (c) ECMWF Apr 15

#ifndef TriangularGrid_H
#define TriangularGrid_H

#include "soyuz/repres/Gridded.h"

class TriangularGrid : public Gridded {
public:

// -- Exceptions
	// None

// -- Contructors

	TriangularGrid(const MIRParametrisation&);

// -- Destructor

	virtual ~TriangularGrid(); // Change to virtual if base class

// -- Convertors
	// None

// -- Operators
	// None

// -- Methods

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

	void print(std::ostream&) const; // Change to virtual if base class

// -- Overridden methods
	// None

// -- Class members
	// None

// -- Class methods
	// None

private:

	TriangularGrid();

// No copy allowed

	TriangularGrid(const TriangularGrid&);
	TriangularGrid& operator=(const TriangularGrid&);

// -- Members


// -- Methods
	// None


// -- Overridden methods

    virtual void fill(grib_info&) const;

// -- Class members
	// None

// -- Class methods
	// None

// -- Friends

	//friend ostream& operator<<(ostream& s,const TriangularGrid& p)
	//	{ p.print(s); return s; }

};

#endif
