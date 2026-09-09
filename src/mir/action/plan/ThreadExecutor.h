// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <iosfwd>

#include "mir/action/plan/Executor.h"


namespace mir::action {


class ThreadExecutor : public Executor {
public:
    // -- Exceptions
    // None

    // -- Constructors

    ThreadExecutor(const std::string& name);

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

private:
    // -- Members
    // None

    // -- Methods
    // None

    // -- Overridden methods

    void print(std::ostream&) const override;

    void execute(context::Context&, const ActionNode&) const override;
    void wait() const override;
    void parametrisation(const param::MIRParametrisation&) override;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


}  // namespace mir::action
