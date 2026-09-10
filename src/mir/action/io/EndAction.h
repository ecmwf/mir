// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/action/plan/Action.h"


namespace mir::output {
class MIROutput;
}  // namespace mir::output


namespace mir::action::io {


class EndAction : public Action {
public:
    // -- Exceptions
    // None

    // -- Constructors

    EndAction(const param::MIRParametrisation&, output::MIROutput&);

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

    output::MIROutput& output() { return output_; }
    const output::MIROutput& output() const { return output_; }

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

private:
    // -- Members

    output::MIROutput& output_;

    // -- Methods
    // None

    // -- Overridden methods

    bool sameAs(const Action&) const override;
    bool isEndAction() const override;

    void print(std::ostream&) const override;
    void custom(std::ostream&) const override;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


}  // namespace mir::action::io
