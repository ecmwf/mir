// File NullFilter.h
// Baudouin Raoult - (c) ECMWF Apr 15

#ifndef NullFilter_H
#define NullFilter_H

#include "Action.h"

class NullFilter : public Action {
public:

// -- Exceptions
	// None

// -- Contructors

	NullFilter(const MIRParametrisation&);

// -- Destructor

	virtual ~NullFilter(); // Change to virtual if base class

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

	NullFilter(const NullFilter&);
	NullFilter& operator=(const NullFilter&);

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

	//friend ostream& operator<<(ostream& s,const NullFilter& p)
	//	{ p.print(s); return s; }

};

#endif
