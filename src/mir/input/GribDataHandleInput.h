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


#ifndef mir_input_GribDataHandleInput_h
#define mir_input_GribDataHandleInput_h

#include "mir/input/GribStreamInput.h"


namespace mir {
namespace input {


class GribDataHandleInput : public GribStreamInput {
public:
    // -- Exceptions
    // None

    // -- Constructors

    GribDataHandleInput(eckit::DataHandle& handle, size_t skip, size_t step);
    GribDataHandleInput(eckit::DataHandle& handle, off_t offset);
    GribDataHandleInput(eckit::DataHandle& handle);

    // -- Destructor

    ~GribDataHandleInput();  // Change to virtual if base class

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

    eckit::DataHandle& handle_;

    // -- Methods
    // None

    // -- Overridden methods

    // From MIRInput
    virtual void print(std::ostream&) const override;  // Change to virtual if base class
    virtual bool sameAs(const MIRInput& other) const;

    // From GribInput
    virtual eckit::DataHandle& dataHandle();

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    // friend ostream& operator<<(ostream& s,const GribDataHandleInput& p)
    // { p.print(s); return s; }
};


}  // namespace input
}  // namespace mir


#endif
