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

#include "mir/action/plan/Action.h"

#include <memory>


namespace mir::stats {
class Distribution;
}  // namespace mir::stats


namespace mir::action::filter {


class AddRandomFilter : public Action {
public:
    // -- Types
    // None

    // -- Exceptions
    // None

    // -- Constructors

    explicit AddRandomFilter(const param::MIRParametrisation&);

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

    std::unique_ptr<stats::Distribution> distribution_;

    // -- Methods
    // None

    // -- Overridden methods

    // From Action
    bool sameAs(const Action&) const override;
    const char* name() const override;
    void print(std::ostream&) const override;
    void execute(context::Context&) const override;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


}  // namespace mir::action::filter
