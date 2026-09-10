// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/output/MIROutput.h"


namespace mir::output {


class EmptyOutput : public MIROutput {
public:
    // -- Exceptions
    // None

    // -- Constructors

    EmptyOutput();
    EmptyOutput(const std::string& path);

    // -- Destructor
    // None

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

    // From MIROutput
    size_t save(const param::MIRParametrisation&, context::Context&) override;
    bool sameAs(const MIROutput&) const override;
    bool sameParametrisation(const param::MIRParametrisation&, const param::MIRParametrisation&) const override;
    bool printParametrisation(std::ostream&, const param::MIRParametrisation&) const override;
    void print(std::ostream&) const override;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


}  // namespace mir::output
