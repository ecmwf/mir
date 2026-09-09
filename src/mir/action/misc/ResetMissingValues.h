// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/action/plan/Action.h"


namespace mir::action {


class ResetMissingValues : public Action {
public:
    // -- Exceptions
    // None

    // -- Constructors

    using Action::Action;

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

    void execute(context::Context&) const override;
    void print(std::ostream&) const override;
    bool sameAs(const Action&) const override;
    const char* name() const override;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


}  // namespace mir::action
