// File GribStreamOutput.h
// Baudouin Raoult - (c) ECMWF Apr 15

#ifndef GribStreamOutput_H
#define GribStreamOutput_H

// namespace outline;

#include "soyuz/output/GribOutput.h"


namespace eckit {
class DataHandle;
}

class GribStreamOutput : public GribOutput {
public:

// -- Exceptions
	// None

// -- Contructors

	GribStreamOutput();

// -- Destructor

	~GribStreamOutput(); // Change to virtual if base class

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

	GribStreamOutput(const GribStreamOutput&);
	GribStreamOutput& operator=(const GribStreamOutput&);

// -- Members

// -- Methods

	virtual eckit::DataHandle& dataHandle() = 0;

// -- Overridden methods
	// From MIROutput

	void out(const void* message, size_t length, bool iterpolated);

// -- Class members
	// None

// -- Class methods
	// None

// -- Friends

	//friend ostream& operator<<(ostream& s,const GribStreamOutput& p)
	//	{ p.print(s); return s; }

};

#endif
