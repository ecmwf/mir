// File MIRConfiguration.h
// Baudouin Raoult - (c) ECMWF Apr 15

#ifndef MIRConfiguration_H
#define MIRConfiguration_H

#include "soyuz/param/MIRParametrisation.h"

#include <string>
#include <map>

class MIRConfiguration : public MIRParametrisation {
public:

// -- Exceptions
	// None

// -- Contructors

	MIRConfiguration();

// -- Destructor

	~MIRConfiguration(); // Change to virtual if base class

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

	// void print(ostream&) const; // Change to virtual if base class

// -- Overridden methods
	// None

// -- Class members
	// None

// -- Class methods
	// None

private:

// No copy allowed

	MIRConfiguration(const MIRConfiguration&);
	MIRConfiguration& operator=(const MIRConfiguration&);

// -- Members

	std::map<std::string, std::string> settings_;

// -- Methods
	// None

// -- Overridden methods

	// From MIRParametrisation
    virtual void print(std::ostream&) const;
    virtual bool get(const std::string&, std::string&) const;

// -- Class members
	// None

// -- Class methods
	// None

// -- Friends

	//friend ostream& operator<<(ostream& s,const MIRConfiguration& p)
	//	{ p.print(s); return s; }

};

#endif
