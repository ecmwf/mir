// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


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
