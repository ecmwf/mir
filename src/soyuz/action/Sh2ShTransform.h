// File Sh2ShTransform.h
// Baudouin Raoult - (c) ECMWF Apr 15

#ifndef Sh2ShTransform_H
#define Sh2ShTransform_H

#include "soyuz/action/Action.h"

class Sh2ShTransform : public Action {
  public:

// -- Exceptions
    // None

// -- Contructors

    Sh2ShTransform(const MIRParametrisation&);

// -- Destructor

    virtual ~Sh2ShTransform(); // Change to virtual if base class

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

    Sh2ShTransform(const Sh2ShTransform&);
    Sh2ShTransform& operator=(const Sh2ShTransform&);

// -- Members

    size_t truncation_;

// -- Methods
    // None

// -- Overridden methods

    virtual void execute(MIRField&) const;

// -- Class members
    // None

// -- Class methods
    // None

// -- Friends

    //friend ostream& operator<<(ostream& s,const Sh2ShTransform& p)
    //	{ p.print(s); return s; }

};

#endif
