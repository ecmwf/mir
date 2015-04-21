// File FrameFilter.h
// Baudouin Raoult - (c) ECMWF Apr 15

#ifndef FrameFilter_H
#define FrameFilter_H

#include "soyuz/action/Action.h"

#include "eckit/filesystem/PathName.h"


class FrameFilter : public Action {
  public:

// -- Exceptions
    // None

// -- Contructors

    FrameFilter(const MIRParametrisation&);

// -- Destructor

    virtual ~FrameFilter(); // Change to virtual if base class

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

    FrameFilter(const FrameFilter&);
    FrameFilter& operator=(const FrameFilter&);

// -- Members

    size_t size_;

// -- Methods
    // None

// -- Overridden methods

    virtual void execute(MIRField&) const;


// -- Class members
    // None

// -- Class methods
    // None

// -- Friends

    //friend ostream& operator<<(ostream& s,const FrameFilter& p)
    //	{ p.print(s); return s; }

};

#endif
