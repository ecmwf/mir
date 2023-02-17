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

    ECMWFStyle(const param::MIRParametrisation&);

    // -- Destructor

    ~ECMWFStyle() override;

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

    virtual void prologue(action::ActionPlan&) const;
    virtual void sh2grid(action::ActionPlan&) const;
    virtual void sh2sh(action::ActionPlan&) const;
    virtual void grid2grid(action::ActionPlan&) const;
    virtual void epilogue(action::ActionPlan&) const;

    // -- Overridden methods

    void print(std::ostream&) const override;

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

    void prepare(action::ActionPlan&, output::MIROutput&) const override;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


}  // namespace mir::key::style
