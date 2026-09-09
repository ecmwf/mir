// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "eckit/io/Buffer.h"

#include "mir/input/GribInput.h"


namespace eckit {
class DataHandle;
}


namespace mir::input {


class GribStreamInput : public GribInput {
public:
    // -- Exceptions
    // None

    // -- Constructors

    GribStreamInput();
    GribStreamInput(size_t skip, size_t step);
    GribStreamInput(off_t offset);

    // -- Destructor

    ~GribStreamInput() override;

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods
    // None

    // -- Overridden methods

    bool next() override;

    // -- Class members
    // None

    // -- Class methods
    // None

protected:
    // -- Members
    size_t skip_;
    size_t step_;
    off_t offset_;

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

    eckit::Buffer buffer_;
    bool first_;


    // -- Methods

    virtual eckit::DataHandle& dataHandle() = 0;

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


}  // namespace mir::input
