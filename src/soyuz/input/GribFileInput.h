// File GribFileInput.h
// Baudouin Raoult - (c) ECMWF Apr 15

#ifndef GribFileInput_H
#define GribFileInput_H

// namespace outline;

#include "soyuz/input/GribStreamInput.h"
#include "eckit/filesystem/PathName.h"


class GribFileInput : public GribStreamInput {
public:

// -- Exceptions
	// None

// -- Contructors

	GribFileInput(const eckit::PathName&);

// -- Destructor

	~GribFileInput(); // Change to virtual if base class

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

	GribFileInput(const GribFileInput&);
	GribFileInput& operator=(const GribFileInput&);

// -- Members

    eckit::PathName path_;
    eckit::DataHandle* handle_;

// -- Methods
	// None

// -- Overridden methods
	// From MIRInput

    virtual void print(std::ostream&) const; // Change to virtual if base class

    // From GribInput

    virtual eckit::DataHandle& dataHandle();


// -- Class members
	// None

// -- Class methods
	// None

// -- Friends

	//friend ostream& operator<<(ostream& s,const GribFileInput& p)
	//	{ p.print(s); return s; }

};

#endif
