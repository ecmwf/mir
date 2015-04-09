
// File Albers.h
// Baudouin Raoult - (c) ECMWF Apr 15

#ifndef Albers_H
#define Albers_H

#include "Gridded.h"

class Albers : public Gridded {
public:

// -- Exceptions
	// None

// -- Contructors

	Albers(const MIRParametrisation&);

// -- Destructor

	virtual ~Albers(); // Change to virtual if base class

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

	Albers();

// No copy allowed

	Albers(const Albers&);
	Albers& operator=(const Albers&);

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

	//friend ostream& operator<<(ostream& s,const Albers& p)
	//	{ p.print(s); return s; }

};

#endif
