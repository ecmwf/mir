/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Pedro Maciel
/// @date Apr 2015


#ifndef GribOutput_H
#define GribOutput_H

#include "soyuz/output/MIROutput.h"


namespace eckit {
class DataHandle;
}


namespace mir {
namespace output {


class GribOutput : public MIROutput {
  public:

// -- Exceptions
    // None

// -- Contructors

    GribOutput();

// -- Destructor

    ~GribOutput(); // Change to virtual if base class

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

    GribOutput(const GribOutput&);
    GribOutput& operator=(const GribOutput&);

// -- Members

// -- Methods

    virtual void out(const void* message, size_t length, bool iterpolated) = 0;

// -- Overridden methods
    // From MIROutput

    virtual void copy(const MIRParametrisation&, mir::input::MIRInput&); // Not iterpolation performed
    virtual void save(const MIRParametrisation&, mir::input::MIRInput&, MIRField&);


// -- Class members
    // None

// -- Class methods
    // None

// -- Friends

    //friend ostream& operator<<(ostream& s,const GribOutput& p)
    // { p.print(s); return s; }

};


}  // namespace output
}  // namespace mir
#endif

