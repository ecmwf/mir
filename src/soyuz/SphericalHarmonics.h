// File SphericalHarmonics.h
// Baudouin Raoult - (c) ECMWF Apr 15

#ifndef SphericalHarmonics_H
#define SphericalHarmonics_H

#include "Representation.h"

class SphericalHarmonics : public Representation {
public:

// -- Exceptions
	// None

// -- Contructors

	SphericalHarmonics(const MIRParametrisation&);

// -- Destructor

	virtual ~SphericalHarmonics(); // Change to virtual if base class

// -- Convertors
	// None

// -- Operators
	// None

// -- Methods

	size_t truncation() const { return truncation_; }

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

	SphericalHarmonics(size_t truncation);

// No copy allowed

	SphericalHarmonics(const SphericalHarmonics&);
	SphericalHarmonics& operator=(const SphericalHarmonics&);

// -- Members

    size_t truncation_;

// -- Methods
	// None


// -- Overridden methods

    virtual void fill(grib_info&) const;
 	virtual Representation* truncate(size_t truncation,
            const std::vector<double>&, std::vector<double>&) const;

// -- Class members
	// None

// -- Class methods
	// None

// -- Friends

	//friend ostream& operator<<(ostream& s,const SphericalHarmonics& p)
	//	{ p.print(s); return s; }

};

#endif
