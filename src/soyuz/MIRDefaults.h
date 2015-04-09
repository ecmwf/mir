// File MIRDefaults.h
// Baudouin Raoult - (c) ECMWF Apr 15

#ifndef MIRDefaults_H
#define MIRDefaults_H

#include "MIRParametrisation.h"

#include <string>
#include <map>

class MIRDefaults : public MIRParametrisation {
public:

// -- Exceptions
	// None

// -- Contructors

	MIRDefaults();

// -- Destructor

	~MIRDefaults(); // Change to virtual if base class

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

	MIRDefaults(const MIRDefaults&);
	MIRDefaults& operator=(const MIRDefaults&);

// -- Members
	std::map<std::string, std::string> defaults_;

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

	//friend ostream& operator<<(ostream& s,const MIRDefaults& p)
	//	{ p.print(s); return s; }

};

#endif
