// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "eckit/types/Fraction.h"


namespace mir::iterator::detail {


class RegularIterator {
public:
    // -- Exceptions
    // None

    // -- Constructors

    RegularIterator(const eckit::Fraction& a, const eckit::Fraction& b, const eckit::Fraction& inc,
                    const eckit::Fraction& ref);

    RegularIterator(const eckit::Fraction& a, const eckit::Fraction& b, const eckit::Fraction& inc,
                    const eckit::Fraction& ref, const eckit::Fraction& period);

    RegularIterator(const RegularIterator&) = delete;
    RegularIterator(RegularIterator&&)      = delete;

    // -- Destructor

    virtual ~RegularIterator() = default;

    // -- Convertors
    // None

    // -- Operators

    void operator=(const RegularIterator&) = delete;
    void operator=(RegularIterator&&)      = delete;

    // -- Methods

    static eckit::Fraction adjust(const eckit::Fraction& target, const eckit::Fraction& inc, bool up);

    const eckit::Fraction& a() const { return a_; }

    const eckit::Fraction& b() const { return b_; }

    const eckit::Fraction& inc() const { return inc_; }

    size_t n() const { return n_; }

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

    eckit::Fraction a_;
    eckit::Fraction b_;
    eckit::Fraction inc_;
    size_t n_;

    // -- Methods
    // None

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


}  // namespace mir::iterator::detail
