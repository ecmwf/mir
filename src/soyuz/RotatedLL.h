
// File RotatedLL.h
// Baudouin Raoult - (c) ECMWF Apr 15

#ifndef RotatedLL_H
#define RotatedLL_H

#include "Gridded.h"

class RotatedLL : public Gridded {
public:

// -- Exceptions
	// None

// -- Contructors

	RotatedLL(const MIRParametrisation&);

// -- Destructor

	virtual ~RotatedLL(); // Change to virtual if base class

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

	RotatedLL();

// No copy allowed

	RotatedLL(const RotatedLL&);
	RotatedLL& operator=(const RotatedLL&);

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

	//friend ostream& operator<<(ostream& s,const RotatedLL& p)
	//	{ p.print(s); return s; }

};

#endif
