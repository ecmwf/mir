// File GribStreamInput.h
// Baudouin Raoult - (c) ECMWF Apr 15

#ifndef GribStreamInput_H
#define GribStreamInput_H

// namespace outline;


#include "soyuz/input/GribInput.h"

#include "eckit/io/Buffer.h"


namespace eckit {
class DataHandle;
}

class GribStreamInput : public GribInput {
  public:

    // -- Exceptions
    // None

    // -- Contructors

    GribStreamInput();

    // -- Destructor

    virtual ~GribStreamInput(); // Change to virtual if base class

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods
    // None

    bool next();


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

    GribStreamInput(const GribStreamInput &);
    GribStreamInput &operator=(const GribStreamInput &);

    // -- Members

    eckit::Buffer buffer_;

    // -- Methods

    virtual eckit::DataHandle &dataHandle() = 0;

    // -- Overridden methods


    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    //friend ostream& operator<<(ostream& s,const GribStreamInput& p)
    //  { p.print(s); return s; }

};

#endif
