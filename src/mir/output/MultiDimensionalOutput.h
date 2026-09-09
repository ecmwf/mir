// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <vector>

#include "mir/output/MIROutput.h"


namespace mir::output {


class MultiDimensionalOutput : public MIROutput {
public:
    // -- Exceptions
    // None

    // -- Constructors

    MultiDimensionalOutput();

    // -- Destructor

    ~MultiDimensionalOutput() override;

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods

    void appendDimensionalOutput(MIROutput*);

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

    std::vector<MIROutput*> dimensions_;

    // -- Methods
    // None

    // -- Overridden methods

    // From MIROutput
    size_t copy(const param::MIRParametrisation&, context::Context&) override;
    size_t save(const param::MIRParametrisation&, context::Context&) override;
    size_t set(const param::MIRParametrisation&, context::Context&) override;
    bool sameAs(const MIROutput&) const override;
    bool sameParametrisation(const param::MIRParametrisation&, const param::MIRParametrisation&) const override;
    bool printParametrisation(std::ostream&, const param::MIRParametrisation&) const override;
    void prepare(const param::MIRParametrisation&, action::ActionPlan&, MIROutput&) override;
    void print(std::ostream&) const override;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
};


}  // namespace mir::output
