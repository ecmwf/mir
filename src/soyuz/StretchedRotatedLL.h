
// File StretchedRotatedLL.h
// Baudouin Raoult - (c) ECMWF Apr 15

#ifndef StretchedRotatedLL_H
#define StretchedRotatedLL_H

#include "Gridded.h"

class StretchedRotatedLL : public Gridded {
public:

// -- Exceptions
	// None

// -- Contructors

	StretchedRotatedLL(const MIRParametrisation&);

// -- Destructor

	virtual ~StretchedRotatedLL(); // Change to virtual if base class

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

	StretchedRotatedLL();

// No copy allowed

	StretchedRotatedLL(const StretchedRotatedLL&);
	StretchedRotatedLL& operator=(const StretchedRotatedLL&);

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

	//friend ostream& operator<<(ostream& s,const StretchedRotatedLL& p)
	//	{ p.print(s); return s; }

};

#endif
