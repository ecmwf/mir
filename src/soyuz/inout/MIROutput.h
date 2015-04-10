// File MIROutput.h
// Baudouin Raoult - (c) ECMWF Apr 15

#ifndef MIROutput_H
#define MIROutput_H

#include <iosfwd>

class MIRParametrisation;
class MIRInput;
class MIRField;

class MIROutput {
public:

// -- Exceptions
	// None

// -- Contructors

	MIROutput();

// -- Destructor

	virtual ~MIROutput(); // Change to virtual if base class

// -- Convertors
	// None

// -- Operators
	// None

// -- Methods

	virtual void copy(const MIRParametrisation&, MIRInput&) = 0; // Not iterpolation performed
	virtual void save(const MIRParametrisation&, MIRInput&, MIRField&) = 0;

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

	MIROutput(const MIROutput&);
	MIROutput& operator=(const MIROutput&);

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

	friend std::ostream& operator<<(std::ostream& s,const MIROutput& p)
		{ p.print(s); return s; }

};

#endif
