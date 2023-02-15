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


#pragma once

#include "mir/input/GribStreamInput.h"


namespace mir::input {


class GribDataHandleInput : public GribStreamInput {
public:
    // -- Exceptions
    // None

    // -- Constructors

    GribDataHandleInput(eckit::DataHandle& handle, size_t skip, size_t step);
    GribDataHandleInput(eckit::DataHandle& handle, off_t offset);
    GribDataHandleInput(eckit::DataHandle& handle);

    // -- Destructor

    ~GribDataHandleInput() override;

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
    void print(std::ostream&) const override;
    bool sameAs(const MIRInput&) const override;

    // From GribInput
    eckit::DataHandle& dataHandle() override;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


}  // namespace mir::input
