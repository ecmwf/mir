// File MIRInput.h
// Baudouin Raoult - (c) ECMWF Apr 15

#ifndef MIRInput_H
#define MIRInput_H

#include <iosfwd>

class MIRParametrisation;
class MIRField;

// Forward declaration only, not need for grib_api
typedef struct grib_handle grib_handle;


class MIRInput {
public:

// -- Exceptions
	// None

// -- Contructors

	MIRInput();

// -- Destructor

	virtual ~MIRInput(); // Change to virtual if base class

// -- Convertors
	// None

// -- Operators
	// None

// -- Methods

    virtual const MIRParametrisation& parametrisation() const = 0;
    virtual MIRField* field() const = 0;
	virtual grib_handle* gribHandle() const;

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

	MIRInput(const MIRInput&);
	MIRInput& operator=(const MIRInput&);

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

	friend std::ostream& operator<<(std::ostream& s,const MIRInput& p)
		{ p.print(s); return s; }

};

#endif
