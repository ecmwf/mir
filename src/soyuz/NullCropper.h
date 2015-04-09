// File NullCropper.h
// Baudouin Raoult - (c) ECMWF Apr 15

#ifndef NullCropper_H
#define NullCropper_H

#include "Action.h"

class NullCropper : public Action {
public:

// -- Exceptions
	// None

// -- Contructors

	NullCropper(const MIRParametrisation&);

// -- Destructor

	virtual ~NullCropper(); // Change to virtual if base class

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

	NullCropper(const NullCropper&);
	NullCropper& operator=(const NullCropper&);

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

	//friend ostream& operator<<(ostream& s,const NullCropper& p)
	//	{ p.print(s); return s; }

};

#endif
