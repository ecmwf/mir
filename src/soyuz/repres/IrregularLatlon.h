
// File IrregularLatlon.h
// Baudouin Raoult - (c) ECMWF Apr 15

#ifndef IrregularLatlon_H
#define IrregularLatlon_H

#include "soyuz/repres/Gridded.h"

class IrregularLatlon : public Gridded {
  public:

// -- Exceptions
    // None

// -- Contructors

    IrregularLatlon(const MIRParametrisation&);

// -- Destructor

    virtual ~IrregularLatlon(); // Change to virtual if base class

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

    IrregularLatlon();

// No copy allowed

    IrregularLatlon(const IrregularLatlon&);
    IrregularLatlon& operator=(const IrregularLatlon&);

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

    //friend ostream& operator<<(ostream& s,const IrregularLatlon& p)
    //	{ p.print(s); return s; }

};

#endif
