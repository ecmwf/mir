
// File StretchedGG.h
// Baudouin Raoult - (c) ECMWF Apr 15

#ifndef StretchedGG_H
#define StretchedGG_H

#include "Gridded.h"

class StretchedGG : public Gridded {
public:

// -- Exceptions
	// None

// -- Contructors

	StretchedGG(const MIRParametrisation&);

// -- Destructor

	virtual ~StretchedGG(); // Change to virtual if base class

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

	StretchedGG();

// No copy allowed

	StretchedGG(const StretchedGG&);
	StretchedGG& operator=(const StretchedGG&);

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

	//friend ostream& operator<<(ostream& s,const StretchedGG& p)
	//	{ p.print(s); return s; }

};

#endif
