// File AreaCropper.h
// Baudouin Raoult - (c) ECMWF Apr 15

#ifndef AreaCropper_H
#define AreaCropper_H

#include "Action.h"

class AreaCropper : public Action {
public:

// -- Exceptions
	// None

// -- Contructors

	AreaCropper(const MIRParametrisation&);

// -- Destructor

	virtual ~AreaCropper(); // Change to virtual if base class

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

	void print(std::ostream&) const; // Change to virtual if base class

// -- Overridden methods
	// None

// -- Class members
	// None

// -- Class methods
	// None

private:

// No copy allowed

	AreaCropper(const AreaCropper&);
	AreaCropper& operator=(const AreaCropper&);

// -- Members

	double north_;
	double west_;
	double south_;
	double east_;

// -- Methods
	// None

// -- Overridden methods

    virtual void execute(MIRField&) const;

// -- Class members
	// None

// -- Class methods
	// None

// -- Friends

	//friend ostream& operator<<(ostream& s,const AreaCropper& p)
	//	{ p.print(s); return s; }

};

#endif
