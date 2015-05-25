/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Tiago Quintino
/// @author Baudouin Raoult
/// @date   May 2015

#include <iostream>
#include <cmath>
#include <limits>

#include "mir/data/MIRFieldStats.h"
#include "eckit/exception/Exceptions.h"


namespace mir {
namespace data {


MIRFieldStats::MIRFieldStats(const std::vector<double>& vs, size_t missing) :
    missing_(missing),
    min_(std::numeric_limits<double>::max()),
    max_(std::numeric_limits<double>::min()),
    mean_(0.),
    sqsum_(0.),
    stdev_(0.) {

    if (vs.size()) {
        double sum = 0.;
        for (std::vector<double>::const_iterator it = vs.begin(); it != vs.end(); ++it ) {
            double v = *it;
            min_ = std::min(v, min_);
            max_ = std::max(v, max_);
            sum += v;
            sqsum_ += v * v;
        }

        mean_ = sum / vs.size();
        stdev_ = std::sqrt(sqsum_ / vs.size() - mean_ * mean_);
    }
}

void MIRFieldStats::print(std::ostream &s) const {
    s << "Stats[min=" << min_
      << ",max=" << max_
      << ",l2norm=" << std::sqrt(sqsum_)
      << ",mean=" << mean_
      << ",stdev=" << stdev_
      << ",missing=" << missing_ << "]";
}

}  // namespace data
}  // namespace mir

