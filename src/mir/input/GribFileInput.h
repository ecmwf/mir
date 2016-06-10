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


#ifndef GribFileInput_H
#define GribFileInput_H

#include "eckit/filesystem/PathName.h"

#include "mir/input/GribStreamInput.h"


namespace mir {
namespace input {


class GribFileInput : public GribStreamInput {
  public:

// -- Exceptions
    // None

// -- Contructors

    GribFileInput(const eckit::PathName&, size_t skip, size_t step);
    GribFileInput(const eckit::PathName&, off_t offset);
    GribFileInput(const eckit::PathName&);

// -- Destructor

    ~GribFileInput(); // Change to virtual if base class

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

    GribFileInput(const GribFileInput&);
    GribFileInput& operator=(const GribFileInput&);

// -- Members

    eckit::PathName path_;
    eckit::DataHandle* handle_;

// -- Methods
    // None

// -- Overridden methods
    // From MIRInput

    virtual void print(std::ostream&) const; // Change to virtual if base class
    virtual bool sameAs(const MIRInput& other) const;

    // From GribInput

    virtual eckit::DataHandle& dataHandle();


// -- Class members
    // None

// -- Class methods
    // None

// -- Friends

    //friend ostream& operator<<(ostream& s,const GribFileInput& p)
    // { p.print(s); return s; }

};


}  // namespace input
}  // namespace mir
#endif

