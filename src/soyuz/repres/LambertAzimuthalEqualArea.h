
// File LambertAzimuthalEqualArea.h
// Baudouin Raoult - (c) ECMWF Apr 15

#ifndef LambertAzimuthalEqualArea_H
#define LambertAzimuthalEqualArea_H

#include "soyuz/repres/Gridded.h"

class LambertAzimuthalEqualArea : public Gridded {
  public:

// -- Exceptions
    // None

// -- Contructors

    LambertAzimuthalEqualArea(const MIRParametrisation&);

// -- Destructor

    virtual ~LambertAzimuthalEqualArea(); // Change to virtual if base class

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

    LambertAzimuthalEqualArea();

// No copy allowed

    LambertAzimuthalEqualArea(const LambertAzimuthalEqualArea&);
    LambertAzimuthalEqualArea& operator=(const LambertAzimuthalEqualArea&);

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

    //friend ostream& operator<<(ostream& s,const LambertAzimuthalEqualArea& p)
    //	{ p.print(s); return s; }

};

#endif
