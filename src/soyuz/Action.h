// File Action.h
// Baudouin Raoult - (c) ECMWF Apr 15

#ifndef Action_H
#define Action_H

#include <string>

class MIRField;
class MIRParametrisation;

class Action {
public:

// -- Exceptions
	// None

// -- Contructors

	Action(const MIRParametrisation& parametrisation);

// -- Destructor

	virtual ~Action(); // Change to virtual if base class

// -- Convertors
	// None

// -- Operators
	// None

// -- Methods

    virtual void execute(MIRField&) const = 0;


// -- Overridden methods
	// None

// -- Class members
	// None

// -- Class methods
	// None

protected:

// -- Members

    const MIRParametrisation& parametrisation_;

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

	Action(const Action&);
	Action& operator=(const Action&);

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

	friend std::ostream& operator<<(std::ostream& s,const Action& p)
		{ p.print(s); return s; }

};

class ActionFactory {
    std::string name_;
    virtual Action* make(const MIRParametrisation&) = 0 ;

protected:

    ActionFactory(const std::string&);
    virtual ~ActionFactory();

public:
    static Action* build(const std::string&, const MIRParametrisation&);

};

template<class T>
class ActionBuilder : public ActionFactory {
    virtual Action* make(const MIRParametrisation& param) { return new T(param); }
public:
    ActionBuilder(const std::string& name) : ActionFactory(name) {}
};


#endif
