
// File AzimuthRange.h
// Baudouin Raoult - (c) ECMWF Apr 15

#ifndef AzimuthRange_H
#define AzimuthRange_H

#include "soyuz/repres/Gridded.h"

class AzimuthRange : public Gridded {
public:

// -- Exceptions
	// None

// -- Contructors

	AzimuthRange(const MIRParametrisation&);

// -- Destructor

	virtual ~AzimuthRange(); // Change to virtual if base class

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

	void print(std::ostream&) const; // Change to virtual if base class

// -- Overridden methods
	// None

// -- Class members
	// None

// -- Class methods
	// None

private:

	AzimuthRange();

// No copy allowed

	AzimuthRange(const AzimuthRange&);
	AzimuthRange& operator=(const AzimuthRange&);

// -- Members


// -- Methods
	// None


// -- Overridden methods

    virtual void fill(grib_info&) const;

// -- Class members
	// None

// -- Class methods
	// None

// -- Friends

	//friend ostream& operator<<(ostream& s,const AzimuthRange& p)
	//	{ p.print(s); return s; }

};

#endif
