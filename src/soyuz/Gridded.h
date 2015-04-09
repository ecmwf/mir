// File Gridded.h
// Baudouin Raoult - (c) ECMWF Apr 15

#ifndef Gridded_H
#define Gridded_H

#include "Representation.h"

class Gridded : public Representation {
public:

// -- Exceptions
	// None

// -- Contructors

	Gridded(const MIRParametrisation&);

// -- Destructor

	virtual ~Gridded(); // Change to virtual if base class

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
  Gridded();

// -- Methods

// -- Overridden methods
	// None

// -- Class members
	// None

// -- Class methods
	// None

private:

// No copy allowed

  Gridded(const Gridded&);
  Gridded& operator=(const Gridded&);

// -- Members

// -- Methods
	// None

// -- Overridden methods

// -- Class members
	// None

// -- Class methods
	// None

// -- Friends

	//friend ostream& operator<<(ostream& s,const Gridded& p)
	//	{ p.print(s); return s; }

};

#endif
