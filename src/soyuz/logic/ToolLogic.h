// File ToolLogic.h
// Baudouin Raoult - (c) ECMWF Apr 15

#ifndef ToolLogic_H
#define ToolLogic_H


#include "soyuz/logic/MIRLogic.h"

class ToolLogic : public MIRLogic {
public:

// -- Exceptions
	// None

// -- Contructors

	ToolLogic(const MIRParametrisation&);

// -- Destructor

	~ToolLogic(); // Change to virtual if base class

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

	virtual void print(std::ostream&) const; // Change to virtual if base class

// -- Overridden methods
	// None

// -- Class members
	// None

// -- Class methods
	// None

private:

// No copy allowed

	ToolLogic(const ToolLogic&);
	ToolLogic& operator=(const ToolLogic&);

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

	// friend std::ostream& operator<<(std::ostream& s, const ToolLogic& p)
	// { p.print(s); return s; }

};

#endif
