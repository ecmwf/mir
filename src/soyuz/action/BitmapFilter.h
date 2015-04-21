// File BitmapFilter.h
// Baudouin Raoult - (c) ECMWF Apr 15

#ifndef BitmapFilter_H
#define BitmapFilter_H

#include "soyuz/action/Action.h"

#include "eckit/filesystem/PathName.h"

class Bitmap;

class BitmapFilter : public Action {
  public:

// -- Exceptions
    // None

// -- Contructors

    BitmapFilter(const MIRParametrisation&);

// -- Destructor

    virtual ~BitmapFilter(); // Change to virtual if base class

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

    BitmapFilter(const BitmapFilter&);
    BitmapFilter& operator=(const BitmapFilter&);

// -- Members

    const Bitmap* bitmap_;

// -- Methods
    // None

// -- Overridden methods

    virtual void execute(MIRField&) const;


// -- Class members
    // None

// -- Class methods
    // None

// -- Friends

    //friend ostream& operator<<(ostream& s,const BitmapFilter& p)
    //	{ p.print(s); return s; }

};

#endif
