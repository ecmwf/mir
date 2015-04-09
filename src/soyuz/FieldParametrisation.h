// File FieldParametrisation.h
// Baudouin Raoult - (c) ECMWF Apr 15

#ifndef FieldParametrisation_H
#define FieldParametrisation_H

#include "MIRParametrisation.h"

class FieldParametrisation : public MIRParametrisation {
public:

// -- Exceptions
	// None

// -- Contructors

	FieldParametrisation();

// -- Destructor

	virtual ~FieldParametrisation(); // Change to virtual if base class

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

	virtual void print(std::ostream&) const = 0; // Change to virtual if base class

// -- Overridden methods
	// From MIRParametrisation
    virtual bool get(const std::string& name, std::string& value) const;

// -- Class members
	// None

// -- Class methods
	// None

private:

// No copy allowed

	FieldParametrisation(const FieldParametrisation&);
	FieldParametrisation& operator=(const FieldParametrisation&);

// -- Members
	// None

// -- Methods
    virtual bool lowLevelGet(const std::string& name, std::string& value) const = 0;

// -- Overridden methods

// -- Class members
	// None

// -- Class methods
	// None

// -- Friends

	friend std::ostream& operator<<(std::ostream& s,const FieldParametrisation& p)
		{ p.print(s); return s; }

};

#endif
