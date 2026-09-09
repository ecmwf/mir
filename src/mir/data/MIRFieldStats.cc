// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/data/MIRFieldStats.h"

#include <cmath>
#include <ostream>


namespace mir::data {


MIRFieldStats::MIRFieldStats() : count_(0), missing_(0), min_(0), max_(0), mean_(0), sqsum_(0), stdev_(0) {}

MIRFieldStats::MIRFieldStats(const MIRValuesVector& vs, size_t missing) :
    count_(vs.size()), missing_(missing), min_(0), max_(0), mean_(0), sqsum_(0), stdev_(0) {

    if (count_ > 0) {

        min_ = max_ = vs[0];

        double sum = 0.;
        for (const auto& v : vs) {
            min_ = std::min(v, min_);
            max_ = std::max(v, max_);
            sum += v;
            sqsum_ += v * v;
        }

        mean_  = sum / double(count_);
        stdev_ = std::sqrt(sqsum_ / double(count_) - mean_ * mean_);
    }
}


void MIRFieldStats::print(std::ostream& s) const {
    s << "["
         "min="
      << min_ << ",max=" << max_ << ",mean=" << mean_ << ",stdev=" << stdev_ << ",l2norm=" << std::sqrt(sqsum_)
      << ",count=" << count_ << ",missing=" << missing_ << ",total=" << count_ + missing_ << "]";
}


double MIRFieldStats::maximum() const {
    return max_;
}


double MIRFieldStats::minimum() const {
    return min_;
}


}  // namespace mir::data
