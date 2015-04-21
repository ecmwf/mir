
// File TransverseMercator.h
// Baudouin Raoult - (c) ECMWF Apr 15

#ifndef TransverseMercator_H
#define TransverseMercator_H

#include "soyuz/repres/Gridded.h"

class TransverseMercator : public Gridded {
  public:

// -- Exceptions
    // None

// -- Contructors

    TransverseMercator(const MIRParametrisation&);

// -- Destructor

    virtual ~TransverseMercator(); // Change to virtual if base class

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

    TransverseMercator();

// No copy allowed

    TransverseMercator(const TransverseMercator&);
    TransverseMercator& operator=(const TransverseMercator&);

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

    //friend ostream& operator<<(ostream& s,const TransverseMercator& p)
    //	{ p.print(s); return s; }

};

#endif
