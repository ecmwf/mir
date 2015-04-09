// File GribOutput.h
// Baudouin Raoult - (c) ECMWF Apr 15

#ifndef GribOutput_H
#define GribOutput_H

// namespace outline;

#include "MIROutput.h"


namespace eckit {
class DataHandle;
}

class GribOutput : public MIROutput {
public:

// -- Exceptions
	// None

// -- Contructors

	GribOutput();

// -- Destructor

	~GribOutput(); // Change to virtual if base class

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

	GribOutput(const GribOutput&);
	GribOutput& operator=(const GribOutput&);

// -- Members

// -- Methods

	virtual eckit::DataHandle& dataHandle() = 0;

// -- Overridden methods
	// From MIROutput

	virtual void copy(const MIRParametrisation&, MIRInput&); // Not iterpolation performed
	virtual void save(const MIRParametrisation&, MIRInput&, MIRField&);


// -- Class members
	// None

// -- Class methods
	// None

// -- Friends

	//friend ostream& operator<<(ostream& s,const GribOutput& p)
	//	{ p.print(s); return s; }

};

#endif
