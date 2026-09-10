// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <memory>
#include <vector>

#include "mir/action/plan/Action.h"


namespace mir::stats {
class Statistics;
}  // namespace mir::stats


namespace mir::action::filter {


class StatisticsFilter : public Action {
public:
    // -- Types
    // None

    // -- Exceptions
    // None

    // -- Constructors

    explicit StatisticsFilter(const param::MIRParametrisation&);

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

    std::vector<std::unique_ptr<stats::Statistics>> statistics_;
    size_t precision_;

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
