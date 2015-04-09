// File DummyOutput.h
// Baudouin Raoult - (c) ECMWF Apr 15

#ifndef DummyOutput_H
#define DummyOutput_H

// namespace outline;

#include "MIROutput.h"


namespace eckit {
class DataHandle;
}

class DummyOutput : public MIROutput {
public:

// -- Exceptions
	// None

// -- Contructors

	DummyOutput();

// -- Destructor

	~DummyOutput(); // Change to virtual if base class

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


// -- Overridden methods
	// None

// -- Class members
	// None

// -- Class methods
	// None

private:

// No copy allowed

	DummyOutput(const DummyOutput&);
	DummyOutput& operator=(const DummyOutput&);

// -- Members

// -- Methods
	// None


// -- Overridden methods
	// From MIROutput
    virtual void print(std::ostream&) const; // Change to virtual if base class

	virtual void copy(const MIRParametrisation&, MIRInput&); // Not iterpolation performed
	virtual void save(const MIRParametrisation&, MIRInput&, MIRField&);


// -- Class members
	// None

// -- Class methods
	// None

// -- Friends

	//friend ostream& operator<<(ostream& s,const DummyOutput& p)
	//	{ p.print(s); return s; }

};

#endif
