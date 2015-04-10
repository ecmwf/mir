
// File ReducedGG.h
// Baudouin Raoult - (c) ECMWF Apr 15

#ifndef ReducedGG_H
#define ReducedGG_H

#include "Gridded.h"

class ReducedGG : public Gridded {
public:

// -- Exceptions
	// None

// -- Contructors

	ReducedGG(const MIRParametrisation&);

// -- Destructor

	virtual ~ReducedGG(); // Change to virtual if base class

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

	ReducedGG();

// No copy allowed

	ReducedGG(const ReducedGG&);
	ReducedGG& operator=(const ReducedGG&);

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

	//friend ostream& operator<<(ostream& s,const ReducedGG& p)
	//	{ p.print(s); return s; }

};

#endif
