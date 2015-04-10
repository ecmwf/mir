
// File PolarStereographic.h
// Baudouin Raoult - (c) ECMWF Apr 15

#ifndef PolarStereographic_H
#define PolarStereographic_H

#include "soyuz/repres/Gridded.h"

class PolarStereographic : public Gridded {
public:

// -- Exceptions
	// None

// -- Contructors

	PolarStereographic(const MIRParametrisation&);

// -- Destructor

	virtual ~PolarStereographic(); // Change to virtual if base class

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

	PolarStereographic();

// No copy allowed

	PolarStereographic(const PolarStereographic&);
	PolarStereographic& operator=(const PolarStereographic&);

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

	//friend ostream& operator<<(ostream& s,const PolarStereographic& p)
	//	{ p.print(s); return s; }

};

#endif
