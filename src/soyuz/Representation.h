// File Representation.h
// Baudouin Raoult - (c) ECMWF Apr 15

#ifndef Representation_H
#define Representation_H

#include <string>
#include <iosfwd>
#include <vector>

class MIRParametrisation;

struct grib_info;

class Representation {
public:

// -- Exceptions
	// None

// -- Contructors

	Representation();

// -- Destructor

	virtual ~Representation(); // Change to virtual if base class

// -- Convertors
	// None

// -- Operators
	// None

// -- Methods

    virtual void fill(grib_info&) const;

    virtual Representation* crop(double north, double west, double south, double east,
            const std::vector<double>&, std::vector<double>&) const;

    virtual size_t frame(std::vector<double> &values, size_t size, double missingValue) const;

    virtual Representation* truncate(size_t truncation,
            const std::vector<double>&, std::vector<double>&) const;

// -- Overridden methods
	// None

// -- Class members
	// None

// -- Class methods
	// None

protected:

// -- Members


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

	Representation(const Representation&);
	Representation& operator=(const Representation&);

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

	friend std::ostream& operator<<(std::ostream& s,const Representation& p)
		{ p.print(s); return s; }

};

class RepresentationFactory {
    std::string name_;
    virtual Representation* make(const MIRParametrisation&) = 0 ;

protected:

    RepresentationFactory(const std::string&);
    virtual ~RepresentationFactory();

public:
    static Representation* build(const MIRParametrisation&);

};

template<class T>
class RepresentationBuilder : public RepresentationFactory {
    virtual Representation* make(const MIRParametrisation& param) { return new T(param); }
public:
    RepresentationBuilder(const std::string& name) : RepresentationFactory(name) {}
};


#endif
