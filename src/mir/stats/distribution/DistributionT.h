// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/stats/Distribution.h"


namespace mir::param {
class MIRParametrisation;
}  // namespace mir::param


namespace mir::stats::distribution {


template <typename DISTRIBUTION>
class DistributionT : public Distribution {
public:
    // -- Exceptions
    // None

    // -- Constructors

    DistributionT(const param::MIRParametrisation&);

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
    // None

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

private:
    // -- Members

    mutable DISTRIBUTION distribution_;

    // -- Methods

    typename DISTRIBUTION::param_type param(const param::MIRParametrisation&) const;
    std::string to_string(typename DISTRIBUTION::param_type&) const;

    // -- Overridden methods

    // From Distribution
    void reset() override;
    double operator()() const override;
    void print(std::ostream&) const override;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


}  // namespace mir::stats::distribution
