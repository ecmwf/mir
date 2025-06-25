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
