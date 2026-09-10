// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


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
