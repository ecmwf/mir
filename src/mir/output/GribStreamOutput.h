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


#ifndef GribStreamOutput_H
#define GribStreamOutput_H

#include "mir/output/GribOutput.h"


namespace eckit {
class DataHandle;
}


namespace mir {
namespace output {


class GribStreamOutput : public GribOutput {
  public:

// -- Exceptions
    // None

// -- Contructors

    GribStreamOutput();

// -- Destructor

    ~GribStreamOutput(); // Change to virtual if base class

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

    GribStreamOutput(const GribStreamOutput&);
    GribStreamOutput& operator=(const GribStreamOutput&);

// -- Members

// -- Methods

    virtual eckit::DataHandle& dataHandle() = 0;

// -- Overridden methods
    // From MIROutput

    void out(const void* message, size_t length, bool iterpolated);

// -- Class members
    // None

// -- Class methods
    // None

// -- Friends

    //friend ostream& operator<<(ostream& s,const GribStreamOutput& p)
    //	{ p.print(s); return s; }

};


}  // namespace output
}  // namespace mir
#endif

