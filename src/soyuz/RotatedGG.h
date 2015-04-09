
// File RotatedGG.h
// Baudouin Raoult - (c) ECMWF Apr 15

#ifndef RotatedGG_H
#define RotatedGG_H

#include "Gridded.h"

class RotatedGG : public Gridded {
public:

// -- Exceptions
	// None

// -- Contructors

	RotatedGG(const MIRParametrisation&);

// -- Destructor

	virtual ~RotatedGG(); // Change to virtual if base class

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

	RotatedGG();

// No copy allowed

	RotatedGG(const RotatedGG&);
	RotatedGG& operator=(const RotatedGG&);

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

	//friend ostream& operator<<(ostream& s,const RotatedGG& p)
	//	{ p.print(s); return s; }

};

#endif
