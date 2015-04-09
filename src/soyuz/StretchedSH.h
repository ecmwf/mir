
// File StretchedSH.h
// Baudouin Raoult - (c) ECMWF Apr 15

#ifndef StretchedSH_H
#define StretchedSH_H

#include "Gridded.h"

class StretchedSH : public Gridded {
public:

// -- Exceptions
	// None

// -- Contructors

	StretchedSH(const MIRParametrisation&);

// -- Destructor

	virtual ~StretchedSH(); // Change to virtual if base class

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

	StretchedSH();

// No copy allowed

	StretchedSH(const StretchedSH&);
	StretchedSH& operator=(const StretchedSH&);

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

	//friend ostream& operator<<(ostream& s,const StretchedSH& p)
	//	{ p.print(s); return s; }

};

#endif
