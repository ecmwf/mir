
// File StretchedLL.h
// Baudouin Raoult - (c) ECMWF Apr 15

#ifndef StretchedLL_H
#define StretchedLL_H

#include "soyuz/repres/Gridded.h"

class StretchedLL : public Gridded {
  public:

// -- Exceptions
    // None

// -- Contructors

    StretchedLL(const MIRParametrisation&);

// -- Destructor

    virtual ~StretchedLL(); // Change to virtual if base class

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

    StretchedLL();

// No copy allowed

    StretchedLL(const StretchedLL&);
    StretchedLL& operator=(const StretchedLL&);

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

    //friend ostream& operator<<(ostream& s,const StretchedLL& p)
    //	{ p.print(s); return s; }

};

#endif
