/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Tiago Quintino
/// @author Baudouin Raoult
/// @author Pedro Maciel
/// @date May 2015


#include "mir/data/MIRFieldStats.h"

#include <iostream>
#include <cmath>


namespace mir {
namespace data {


MIRFieldStats::MIRFieldStats():
    count_(0),
    missing_(0),
    min_(0),
    max_(0),
    mean_(0),
    sqsum_(0),
    stdev_(0) {
}

MIRFieldStats::MIRFieldStats(const MIRValuesVector& vs, size_t missing) :
    count_(vs.size()),
    missing_(missing),
    min_(0),
    max_(0),
    mean_(0),
    sqsum_(0),
    stdev_(0) {

    if (count_) {

        min_ = max_ = vs[0];

        double sum = 0.;
        for (auto& v : vs) {
            min_ = std::min(v, min_);
            max_ = std::max(v, max_);
            sum += v;
            sqsum_ += v * v;
        }

        mean_ = sum / count_;
        stdev_ = std::sqrt(sqsum_ / count_ - mean_ * mean_);
    }
}


void MIRFieldStats::print(std::ostream &s) const {
    s << "["
          "min=" << min_
      << ",max=" << max_
      << ",mean=" << mean_
      << ",stdev=" << stdev_
      << ",l2norm=" << std::sqrt(sqsum_)
      << ",count=" << count_
      << ",missing=" << missing_
      << ",total=" << count_ + missing_
      << "]";
}


double MIRFieldStats::maximum() const {
    return max_;
}


double MIRFieldStats::minimum() const {
    return min_;
}


}  // namespace data
}  // namespace mir

