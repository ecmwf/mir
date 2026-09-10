// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <map>
#include <memory>

#include "mir/action/plan/Action.h"


namespace mir::util {
class Formula;
}  // namespace mir::util


namespace mir::action {


class FormulaAction : public Action {
public:
    // -- Exceptions
    // None

    // -- Constructors

    explicit FormulaAction(const param::MIRParametrisation&);

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

    void print(std::ostream&) const override;

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

private:
    // -- Members

    std::unique_ptr<util::Formula> formula_;
    std::map<std::string, long> metadata_;

    // -- Methods
    // None

    // -- Overridden methods

    void execute(context::Context&) const override;
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
