// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/output/GribOutput.h"


namespace eckit {
class DataHandle;
}


namespace mir::output {


class GribStreamOutput : public GribOutput {
public:
    // -- Exceptions
    // None

    // -- Constructors

    GribStreamOutput();

    // -- Destructor

    ~GribStreamOutput() override;

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
    // -- Members

    // -- Methods

    virtual eckit::DataHandle& dataHandle() = 0;

    // -- Overridden methods
    // From MIROutput

    void out(const void* message, size_t length, bool interpolated) override;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


}  // namespace mir::output
