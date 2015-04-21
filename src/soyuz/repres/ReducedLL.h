
// File ReducedLL.h
// Baudouin Raoult - (c) ECMWF Apr 15

#ifndef ReducedLL_H
#define ReducedLL_H

#include "soyuz/repres/Gridded.h"

class ReducedLL : public Gridded {
  public:

// -- Exceptions
    // None

// -- Contructors

    ReducedLL(const MIRParametrisation&);

// -- Destructor

    virtual ~ReducedLL(); // Change to virtual if base class

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

    ReducedLL();

// No copy allowed

    ReducedLL(const ReducedLL&);
    ReducedLL& operator=(const ReducedLL&);

// -- Members


// -- Methods
    // None


// -- Overridden methods

    virtual void fill(grib_info&) const;

// -- Class members
    // None

// -- Class methods
    // None

// -- Friends

    //friend ostream& operator<<(ostream& s,const ReducedLL& p)
    //	{ p.print(s); return s; }

};

#endif
