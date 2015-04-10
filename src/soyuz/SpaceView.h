
// File SpaceView.h
// Baudouin Raoult - (c) ECMWF Apr 15

#ifndef SpaceView_H
#define SpaceView_H

#include "Gridded.h"

class SpaceView : public Gridded {
public:

// -- Exceptions
	// None

// -- Contructors

	SpaceView(const MIRParametrisation&);

// -- Destructor

	virtual ~SpaceView(); // Change to virtual if base class

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

	SpaceView();

// No copy allowed

	SpaceView(const SpaceView&);
	SpaceView& operator=(const SpaceView&);

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

	//friend ostream& operator<<(ostream& s,const SpaceView& p)
	//	{ p.print(s); return s; }

};

#endif
