// File ECMWFLogic.h
// Baudouin Raoult - (c) ECMWF Apr 15

#ifndef ECMWFLogic_H
#define ECMWFLogic_H


#include "soyuz/logic/MIRLogic.h"

class ECMWFLogic : public MIRLogic {
public:

// -- Exceptions
	// None

// -- Contructors

	ECMWFLogic(const MIRParametrisation&);

// -- Destructor

	~ECMWFLogic(); // Change to virtual if base class

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

	// void print(std::ostream&) const; // Change to virtual if base class

// -- Overridden methods
	// None

// -- Class members
	// None

// -- Class methods
	// None

private:

// No copy allowed

	ECMWFLogic(const ECMWFLogic&);
	ECMWFLogic& operator=(const ECMWFLogic&);

// -- Members

    virtual void prepare(std::vector<std::auto_ptr<Action> >&) const;

// -- Methods
	// None

// -- Overridden methods
	// None

// -- Class members
	// None

// -- Class methods
	// None

// -- Friends

	// friend std::ostream& operator<<(std::ostream& s, const ECMWFLogic& p)
	// { p.print(s); return s; }

};

#endif
