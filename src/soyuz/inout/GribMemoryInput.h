// File GribMemoryInput.h
// Baudouin Raoult - (c) ECMWF Apr 15

#ifndef GribMemoryInput_H
#define GribMemoryInput_H

// namespace outline;


#include "GribInput.h"

#include "eckit/io/Buffer.h"


namespace eckit {
class DataHandle;
}

class GribMemoryInput : public GribInput {
  public:

    // -- Exceptions
    // None

    // -- Contructors

    GribMemoryInput(const char* message, size_t length);

    // -- Destructor

    virtual ~GribMemoryInput(); // Change to virtual if base class

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


    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

  private:

    // No copy allowed

    GribMemoryInput(const GribMemoryInput &);
    GribMemoryInput &operator=(const GribMemoryInput &);

    // -- Members


    // -- Methods

    // -- Overridden methods

    virtual void print(std::ostream&) const;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    //friend ostream& operator<<(ostream& s,const GribMemoryInput& p)
    //  { p.print(s); return s; }

};

#endif
