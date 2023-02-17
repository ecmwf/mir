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
