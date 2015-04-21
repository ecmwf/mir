
// File StretchedRotatedSH.h
// Baudouin Raoult - (c) ECMWF Apr 15

#ifndef StretchedRotatedSH_H
#define StretchedRotatedSH_H

#include "soyuz/repres/Gridded.h"

class StretchedRotatedSH : public Gridded {
  public:

// -- Exceptions
    // None

// -- Contructors

    StretchedRotatedSH(const MIRParametrisation&);

// -- Destructor

    virtual ~StretchedRotatedSH(); // Change to virtual if base class

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

    StretchedRotatedSH();

// No copy allowed

    StretchedRotatedSH(const StretchedRotatedSH&);
    StretchedRotatedSH& operator=(const StretchedRotatedSH&);

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

    //friend ostream& operator<<(ostream& s,const StretchedRotatedSH& p)
    //	{ p.print(s); return s; }

};

#endif
