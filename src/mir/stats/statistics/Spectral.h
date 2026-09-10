// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/stats/Statistics.h"


namespace mir::stats::statistics {


/**
 * @brief Calculate spectral statistics on a MIRField
 */
class Spectral : public Statistics {
public:
    // -- Exceptions
    // None

    // -- Constructors

    Spectral(const param::MIRParametrisation&);

    // -- Destructor

    ~Spectral() override;

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods

    void reset();
    double mean() const;
    double variance() const;
    double standardDeviation() const;
    double enorm() const;

    // -- Overridden methods

    void execute(const data::MIRField&) override;

    // -- Class members
    // None

    // -- Class methods
    // None

private:
    // -- Members

    double mean_;
    double variance_;
    double stddev_;
    double enorm_;

    // -- Methods
    // None

    // -- Overridden methods

    void print(std::ostream&) const override;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


}  // namespace mir::stats::statistics
