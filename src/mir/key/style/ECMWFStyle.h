// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/key/style/MIRStyle.h"


namespace mir::key::style {


class ECMWFStyle : public MIRStyle {
public:
    // -- Exceptions
    // None

    // -- Constructors

    explicit ECMWFStyle(const param::MIRParametrisation&);

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

    void prologue(action::ActionPlan&) const;

    void sh2grid(action::ActionPlan&) const;
    void sh2grid_compatible(action::ActionPlan& plan) const;
    void sh2sh(action::ActionPlan&) const;
    void grid2grid(action::ActionPlan&) const;

    void epilogue(action::ActionPlan&) const;

    // -- Overridden methods

    void print(std::ostream&) const override;

    // -- Class members
    // None

    // -- Class methods
    // None

private:
    // -- Members

    bool sh2gridCompatible_;

    // -- Methods
    // None

    // -- Overridden methods

    void prepare(action::ActionPlan&, output::MIROutput&) const override;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


}  // namespace mir::key::style
