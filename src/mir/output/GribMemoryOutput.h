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

#include "mir/output/GribOutput.h"


namespace mir::output {


class GribMemoryOutput : public GribOutput {
public:
    // -- Exceptions
    // None

    // -- Constructors

    GribMemoryOutput(void* message, size_t size);

    // -- Destructor

    ~GribMemoryOutput() override;

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods
    // None

    size_t length() const { return length_; }


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

    void* message_;
    size_t size_;
    size_t length_;

    // -- Methods

    // -- Overridden methods

    void out(const void* message, size_t length, bool interpolated) override;
    void print(std::ostream&) const override;
    bool sameAs(const MIROutput&) const override;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


}  // namespace mir::output
