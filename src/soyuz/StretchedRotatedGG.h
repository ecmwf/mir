
// File StretchedRotatedGG.h
// Baudouin Raoult - (c) ECMWF Apr 15

#ifndef StretchedRotatedGG_H
#define StretchedRotatedGG_H

#include "Gridded.h"

class StretchedRotatedGG : public Gridded {
public:

// -- Exceptions
	// None

// -- Contructors

	StretchedRotatedGG(const MIRParametrisation&);

// -- Destructor

	virtual ~StretchedRotatedGG(); // Change to virtual if base class

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

	StretchedRotatedGG();

// No copy allowed

	StretchedRotatedGG(const StretchedRotatedGG&);
	StretchedRotatedGG& operator=(const StretchedRotatedGG&);

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

	//friend ostream& operator<<(ostream& s,const StretchedRotatedGG& p)
	//	{ p.print(s); return s; }

};

#endif
