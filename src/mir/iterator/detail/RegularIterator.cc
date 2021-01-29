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


#include "mir/iterator/detail/RegularIterator.h"

#include "mir/util/Exceptions.h"


namespace mir {
namespace iterator {
namespace detail {


RegularIterator::RegularIterator(const eckit::Fraction& a, const eckit::Fraction& b, const eckit::Fraction& inc,
                                 const eckit::Fraction& ref) :
    inc_(inc) {
    ASSERT(a <= b);
    ASSERT(inc >= 0);

    if (inc_ == 0) {

        b_ = a_ = a;
        n_      = 1;
    }
    else {

        auto shift = (ref / inc_).decimalPart() * inc;
        a_         = shift + adjust(a - shift, inc_, true);

        if (b == a) {
            b_ = a_;
        }
        else {

            auto c = shift + adjust(b - shift, inc_, false);
            c      = a_ + ((c - a_) / inc_).integralPart() * inc_;
            b_     = c < a_ ? a_ : c;
        }

        n_ = size_t(((b_ - a_) / inc_).integralPart() + 1);
    }
    ASSERT(a_ <= b_);
    ASSERT(n_ >= 1);
}


RegularIterator::RegularIterator(const eckit::Fraction& a, const eckit::Fraction& b, const eckit::Fraction& inc,
                                 const eckit::Fraction& ref, const eckit::Fraction& period) :
    RegularIterator(a, b, inc, ref) {
    ASSERT(period > 0);

    if ((n_ - 1) * inc_ >= period) {
        n_ -= 1;
        ASSERT(n_ * inc_ == period || (n_ - 1) * inc_ < period);

        b_ = a_ + (n_ - 1) * inc_;
    }
}


eckit::Fraction RegularIterator::adjust(const eckit::Fraction& target, const eckit::Fraction& inc, bool up) {
    ASSERT(inc > 0);

    auto r = target / inc;
    auto n = r.integralPart();

    if (!r.integer() && (r > 0) == up) {
        n += (up ? 1 : -1);
    }

    return (n * inc);
}


}  // namespace detail
}  // namespace iterator
}  // namespace mir
