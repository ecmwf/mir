// File GribFileOutput.h
// Baudouin Raoult - (c) ECMWF Apr 15

#ifndef GribFileOutput_H
#define GribFileOutput_H

// namespace outline;

#include "soyuz/output/GribStreamOutput.h"
#include "eckit/filesystem/PathName.h"


class GribFileOutput : public GribStreamOutput {
public:

// -- Exceptions
	// None

// -- Contructors

	GribFileOutput(const eckit::PathName&);

// -- Destructor

	~GribFileOutput(); // Change to virtual if base class

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

	GribFileOutput(const GribFileOutput&);
	GribFileOutput& operator=(const GribFileOutput&);

// -- Members

	eckit::PathName path_;
    eckit::DataHandle* handle_;

// -- Methods
	// None

// -- Overridden methods
	// From MIROutput

    virtual void print(std::ostream&) const; // Change to virtual if base class

    // From GribInput

    virtual eckit::DataHandle& dataHandle();

// -- Class members
	// None

// -- Class methods
	// None

// -- Friends

	//friend ostream& operator<<(ostream& s,const GribFileOutput& p)
	//	{ p.print(s); return s; }

};

#endif
