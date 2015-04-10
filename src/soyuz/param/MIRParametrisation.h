// File MIRParametrisation.h
// Baudouin Raoult - (c) ECMWF Apr 15

#ifndef MIRParametrisation_H
#define MIRParametrisation_H

#include <string>
#include <iosfwd>

class MIRParametrisation {
public:

// -- Exceptions
	// None

// -- Contructors

	MIRParametrisation();

// -- Destructor

	virtual ~MIRParametrisation(); // Change to virtual if base class

// -- Convertors
	// None

// -- Operators
	// None

// -- Methods
	// None

	virtual bool has(const std::string& name) const;
    virtual bool get(const std::string& name, std::string& value) const = 0;

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
	// None

// -- Class members
	// None

// -- Class methods
	// None

private:

// No copy allowed

	MIRParametrisation(const MIRParametrisation&);
	MIRParametrisation& operator=(const MIRParametrisation&);

// -- Members
	// None

// -- Methods
	// None

// -- Overridden methods
	// None

// -- Class members
	// None

// -- Class methods
	// None

// -- Friends

	friend std::ostream& operator<<(std::ostream& s,const MIRParametrisation& p)
		{ p.print(s); return s; }

};

#endif
