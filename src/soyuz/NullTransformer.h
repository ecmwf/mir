// File NullTransformer.h
// Baudouin Raoult - (c) ECMWF Apr 15

#ifndef NullTransformer_H
#define NullTransformer_H

#include "Action.h"

class NullTransformer : public Action {
public:

// -- Exceptions
	// None

// -- Contructors

	NullTransformer(const MIRParametrisation&);

// -- Destructor

	virtual ~NullTransformer(); // Change to virtual if base class

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

	NullTransformer(const NullTransformer&);
	NullTransformer& operator=(const NullTransformer&);

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

	//friend ostream& operator<<(ostream& s,const NullTransformer& p)
	//	{ p.print(s); return s; }

};

#endif
