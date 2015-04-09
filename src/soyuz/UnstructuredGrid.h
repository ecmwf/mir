
// File UnstructuredGrid.h
// Baudouin Raoult - (c) ECMWF Apr 15

#ifndef UnstructuredGrid_H
#define UnstructuredGrid_H

#include "Gridded.h"

class UnstructuredGrid : public Gridded {
public:

// -- Exceptions
	// None

// -- Contructors

	UnstructuredGrid(const MIRParametrisation&);

// -- Destructor

	virtual ~UnstructuredGrid(); // Change to virtual if base class

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

	UnstructuredGrid();

// No copy allowed

	UnstructuredGrid(const UnstructuredGrid&);
	UnstructuredGrid& operator=(const UnstructuredGrid&);

// -- Members


// -- Methods
	// None


// -- Overridden methods

    virtual void fill(grib_spec&) const;

// -- Class members
	// None

// -- Class methods
	// None

// -- Friends

	//friend ostream& operator<<(ostream& s,const UnstructuredGrid& p)
	//	{ p.print(s); return s; }

};

#endif
