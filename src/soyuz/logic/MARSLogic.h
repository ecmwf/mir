// File MARSLogic.h
// Baudouin Raoult - (c) ECMWF Apr 15

#ifndef MARSLogic_H
#define MARSLogic_H

#include "soyuz/logic/MIRLogic.h"


class MARSLogic : public MIRLogic {
public:

// -- Exceptions
	// None

// -- Contructors

	MARSLogic(const MIRParametrisation&);

// -- Destructor

	~MARSLogic(); // Change to virtual if base class

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

// No copy allowed

	MARSLogic(const MARSLogic&);
	MARSLogic& operator=(const MARSLogic&);

// -- Members

// -- Methods
	// None

// -- Overridden methods

    virtual void prepare(std::vector<std::auto_ptr<Action> >&) const;

// -- Class members
	// None

// -- Class methods
	// None

// -- Friends

	// friend std::ostream& operator<<(std::ostream& s, const MARSLogic& p)
	// { p.print(s); return s; }

};

#endif
