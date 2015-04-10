// File Method.h
// Baudouin Raoult - (c) ECMWF Apr 15

#ifndef Method_H
#define Method_H

#include <string>
#include <iosfwd>

class MIRParametrisation;

class Method {
public:

// -- Exceptions
	// None

// -- Contructors

	Method();

// -- Destructor

	virtual ~Method(); // Change to virtual if base class

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

	Method(const Method&);
	Method& operator=(const Method&);

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

	friend std::ostream& operator<<(std::ostream& s,const Method& p)
		{ p.print(s); return s; }

};

class MethodFactory {
    std::string name_;
    virtual Method* make(const MIRParametrisation&) = 0 ;

protected:

    MethodFactory(const std::string&);
    virtual ~MethodFactory();

public:
    static Method* build(const MIRParametrisation&);

};

template<class T>
class MethodBuilder : public MethodFactory {
    virtual Method* make(const MIRParametrisation& param) { return new T(param); }
public:
    MethodBuilder(const std::string& name) : MethodFactory(name) {}
};


#endif
