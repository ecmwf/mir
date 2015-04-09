
// File Mercator.h
// Baudouin Raoult - (c) ECMWF Apr 15

#ifndef Mercator_H
#define Mercator_H

#include "Gridded.h"

class Mercator : public Gridded {
public:

// -- Exceptions
	// None

// -- Contructors

	Mercator(const MIRParametrisation&);

// -- Destructor

	virtual ~Mercator(); // Change to virtual if base class

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

	Mercator();

// No copy allowed

	Mercator(const Mercator&);
	Mercator& operator=(const Mercator&);

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

	//friend ostream& operator<<(ostream& s,const Mercator& p)
	//	{ p.print(s); return s; }

};

#endif
