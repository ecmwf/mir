
// File RotatedSH.h
// Baudouin Raoult - (c) ECMWF Apr 15

#ifndef RotatedSH_H
#define RotatedSH_H

#include "Gridded.h"

class RotatedSH : public Gridded {
public:

// -- Exceptions
	// None

// -- Contructors

	RotatedSH(const MIRParametrisation&);

// -- Destructor

	virtual ~RotatedSH(); // Change to virtual if base class

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

	RotatedSH();

// No copy allowed

	RotatedSH(const RotatedSH&);
	RotatedSH& operator=(const RotatedSH&);

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

	//friend ostream& operator<<(ostream& s,const RotatedSH& p)
	//	{ p.print(s); return s; }

};

#endif
