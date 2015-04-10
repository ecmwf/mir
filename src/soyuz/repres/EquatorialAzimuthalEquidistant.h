
// File EquatorialAzimuthalEquidistant.h
// Baudouin Raoult - (c) ECMWF Apr 15

#ifndef EquatorialAzimuthalEquidistant_H
#define EquatorialAzimuthalEquidistant_H

#include "soyuz/repres/Gridded.h"

class EquatorialAzimuthalEquidistant : public Gridded {
public:

// -- Exceptions
	// None

// -- Contructors

	EquatorialAzimuthalEquidistant(const MIRParametrisation&);

// -- Destructor

	virtual ~EquatorialAzimuthalEquidistant(); // Change to virtual if base class

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

	EquatorialAzimuthalEquidistant();

// No copy allowed

	EquatorialAzimuthalEquidistant(const EquatorialAzimuthalEquidistant&);
	EquatorialAzimuthalEquidistant& operator=(const EquatorialAzimuthalEquidistant&);

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

	//friend ostream& operator<<(ostream& s,const EquatorialAzimuthalEquidistant& p)
	//	{ p.print(s); return s; }

};

#endif
