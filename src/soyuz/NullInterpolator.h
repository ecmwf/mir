// File NullInterpolator.h
// Baudouin Raoult - (c) ECMWF Apr 15

#ifndef NullInterpolator_H
#define NullInterpolator_H

#include "Action.h"

class NullInterpolator : public Action {
public:

// -- Exceptions
	// None

// -- Contructors

	NullInterpolator(const MIRParametrisation&);

// -- Destructor

	virtual ~NullInterpolator(); // Change to virtual if base class

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

	NullInterpolator(const NullInterpolator&);
	NullInterpolator& operator=(const NullInterpolator&);

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

	//friend ostream& operator<<(ostream& s,const NullInterpolator& p)
	//	{ p.print(s); return s; }

};

#endif
