// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/stats/Statistics.h"
#include "mir/stats/detail/Counter.h"


namespace mir::stats::statistics {


class GridBoxIntegral : public Statistics, detail::Counter {
public:
    // -- Exceptions
    // None

    // -- Constructors

    GridBoxIntegral(const param::MIRParametrisation&);

    // -- Destructor
    // None

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods

    void reset();

    double integral() const { return integral_; }

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

private:
    // -- Members

    double integral_;

    // -- Methods
    // None

    // -- Overridden methods

    void execute(const data::MIRField&) override;
    void print(std::ostream&) const override;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


}  // namespace mir::stats::statistics
