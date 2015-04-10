// File Sh2GriddedTransform.h
// Baudouin Raoult - (c) ECMWF Apr 15

#ifndef Sh2GriddedTransform_H
#define Sh2GriddedTransform_H

#include "Action.h"

class Sh2GriddedTransform : public Action {
public:

// -- Exceptions
	// None

// -- Contructors

	Sh2GriddedTransform(const MIRParametrisation&);

// -- Destructor

	virtual ~Sh2GriddedTransform(); // Change to virtual if base class

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

	Sh2GriddedTransform(const Sh2GriddedTransform&);
	Sh2GriddedTransform& operator=(const Sh2GriddedTransform&);

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

	//friend ostream& operator<<(ostream& s,const Sh2GriddedTransform& p)
	//	{ p.print(s); return s; }

};

#endif
