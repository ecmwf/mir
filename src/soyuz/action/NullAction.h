// File NullAction.h
// Baudouin Raoult - (c) ECMWF Apr 15

#ifndef NullAction_H
#define NullAction_H

#include "soyuz/action/Action.h"

class NullAction : public Action {
  public:

// -- Exceptions
    // None

// -- Contructors

    NullAction(const MIRParametrisation&);

// -- Destructor

    virtual ~NullAction(); // Change to virtual if base class

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

    NullAction(const NullAction&);
    NullAction& operator=(const NullAction&);

// -- Members
    // None

// -- Methods
    // None

// -- Overridden methods

    virtual void execute(MIRField&) const;

// -- Class members
    // None

// -- Class methods
    // None

// -- Friends

    //friend ostream& operator<<(ostream& s,const NullAction& p)
    //	{ p.print(s); return s; }

};

#endif
