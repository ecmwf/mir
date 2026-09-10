// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/stats/Comparator.h"


namespace mir::stats::comparator {


/**
 * @brief Calculate spectral statistics on a MIRField
 */
class Spectral : public Comparator {
public:
    // -- Exceptions
    // None

    // -- Constructors

    Spectral(const param::MIRParametrisation&, const param::MIRParametrisation&);

    // -- Destructor

    ~Spectral() override;

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods

    void reset();
    double meanDiff() const;
    double enormDiff() const;

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

private:
    // -- Members

    double meanDiffMax_;
    double enormDiffMax_;

    double meanDiff_;
    double enormDiff_;
    std::string stats_;

    // -- Methods
    // None

    // -- Overridden methods

    std::string execute(const data::MIRField&, const data::MIRField&) override;
    void print(std::ostream&) const override;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


}  // namespace mir::stats::comparator
