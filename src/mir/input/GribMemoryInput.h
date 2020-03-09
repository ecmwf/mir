/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 *
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#ifndef mir_input_GribMemoryInput_h
#define mir_input_GribMemoryInput_h

#include "mir/input/GribInput.h"


namespace mir {
namespace input {


class GribMemoryInput : public GribInput {
public:
    // -- Exceptions
    // None

    // -- Constructors

    GribMemoryInput(const void* message, size_t length);

    // -- Destructor

    virtual ~GribMemoryInput();  // Change to virtual if base class

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
    // None

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

private:
    // -- Members
    // None

    // -- Methods
    // None

    // -- Overridden methods

    virtual void print(std::ostream&) const;
    virtual bool sameAs(const MIRInput& other) const;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    // friend ostream& operator<<(ostream& s,const GribMemoryInput& p)
    //  { p.print(s); return s; }
};


}  // namespace input
}  // namespace mir


#endif
