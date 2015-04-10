// File MIRJob.h
// Baudouin Raoult - (c) ECMWF Apr 15

#ifndef MIRJob_H
#define MIRJob_H

#include <string>
#include <map>

// namespace outline;

#include "eckit/memory/NonCopyable.h"

#include "soyuz/param/MIRParametrisation.h"

class MIRInput;
class MIROutput;

class MIRJob : public MIRParametrisation, public eckit::NonCopyable {
public:

// -- Exceptions
	// None

// -- Contructors

	MIRJob();

// -- Destructor

	virtual ~MIRJob();

// -- Convertors
	// None

// -- Operators
	// None

// -- Methods

    void execute(MIRInput&, MIROutput&) const;

    void set(const std::string&, const std::string&);

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


// -- Class members
	// None

// -- Class methods
	// None

private:


// -- Members

    std::map<std::string, std::string> settings_;

// -- Methods

    bool matches(const MIRParametrisation&) const;

// -- Overridden methods

    // From MIRParametrisation

    virtual void print(std::ostream&) const;
	virtual bool get(const std::string&, std::string&) const;

// -- Class members
	// None

// -- Class methods
	// None

// -- Friends


};

#endif
