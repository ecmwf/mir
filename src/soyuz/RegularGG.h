
// File RegularGG.h
// Baudouin Raoult - (c) ECMWF Apr 15

#ifndef RegularGG_H
#define RegularGG_H

#include "Gridded.h"

class RegularGG : public Gridded {
public:

// -- Exceptions
	// None

// -- Contructors

	RegularGG(const MIRParametrisation&);

// -- Destructor

	virtual ~RegularGG(); // Change to virtual if base class

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

	RegularGG();

// No copy allowed

	RegularGG(const RegularGG&);
	RegularGG& operator=(const RegularGG&);

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

	//friend ostream& operator<<(ostream& s,const RegularGG& p)
	//	{ p.print(s); return s; }

};

#endif
