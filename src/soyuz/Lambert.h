
// File Lambert.h
// Baudouin Raoult - (c) ECMWF Apr 15

#ifndef Lambert_H
#define Lambert_H

#include "Gridded.h"

class Lambert : public Gridded {
public:

// -- Exceptions
	// None

// -- Contructors

	Lambert(const MIRParametrisation&);

// -- Destructor

	virtual ~Lambert(); // Change to virtual if base class

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

	Lambert();

// No copy allowed

	Lambert(const Lambert&);
	Lambert& operator=(const Lambert&);

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

	//friend ostream& operator<<(ostream& s,const Lambert& p)
	//	{ p.print(s); return s; }

};

#endif
