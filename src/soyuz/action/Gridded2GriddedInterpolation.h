// File Gridded2GriddedInterpolation.h
// Baudouin Raoult - (c) ECMWF Apr 15

#ifndef Gridded2GriddedInterpolation_H
#define Gridded2GriddedInterpolation_H

#include "soyuz/action/Action.h"

class Gridded2GriddedInterpolation : public Action {
public:

// -- Exceptions
	// None

// -- Contructors

	Gridded2GriddedInterpolation(const MIRParametrisation&);

// -- Destructor

	virtual ~Gridded2GriddedInterpolation(); // Change to virtual if base class

// -- Convertors
	// None

// -- Operators
	// None

// -- Methods
	// None

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

// No copy allowed

	Gridded2GriddedInterpolation(const Gridded2GriddedInterpolation&);
	Gridded2GriddedInterpolation& operator=(const Gridded2GriddedInterpolation&);

// -- Members
	// None

// -- Methods
	// None

// -- Overridden methods

    virtual void execute(MIRField&) const;

// -- Class members
	// None

// -- Class methods
	// None

// -- Friends

	//friend ostream& operator<<(ostream& s,const Gridded2GriddedInterpolation& p)
	//	{ p.print(s); return s; }

};

#endif
