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


#ifndef GribMemoryInput_H
#define GribMemoryInput_H


#include "eckit/io/Buffer.h"

#include "mir/input/GribInput.h"


namespace eckit {
class DataHandle;
}


namespace mir {
namespace input {


class GribMemoryInput : public GribInput {
  public:

    // -- Exceptions
    // None

    // -- Contructors

    GribMemoryInput(const void* message, size_t length);

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


}  // namespace input
}  // namespace mir
#endif

