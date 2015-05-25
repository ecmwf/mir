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


MIRFieldStats::MIRFieldStats() :
    min(std::numeric_limits<double>::max()),
    max(std::numeric_limits<double>::min()),
    mean(0.),
    sqsum(0.),
    stdev(0.),
    ready(false)
{}

MIRFieldStats::MIRFieldStats(const std::vector<double>& vs) :
    min(std::numeric_limits<double>::max()),
    max(std::numeric_limits<double>::min()),
    mean(0.),
    sqsum(0.),
    stdev(0.),
    ready(false)
{
    compute(vs);
}

void MIRFieldStats::compute(const std::vector<double>& vs)
{
    ASSERT(!ready); // Warning, calling compute twice will lead to wrong resusts


    double sum = 0.;
    for(std::vector<double>::const_iterator it = vs.begin(); it != vs.end(); ++it )
    {
        double v = *it;
        min = std::min(v,min);
        max = std::max(v,max);
        sum += v;
        sqsum += v*v;
    }

    mean = sum / vs.size();
    stdev = std::sqrt(sqsum / vs.size() - mean * mean);
    ready = true;
}

void MIRFieldStats::print(std::ostream &s) const
{
    s << "Stats[min=" << min
      << ",max=" << max
      << ",l2norm=" << std::sqrt(sqsum)
      << ",mean=" << mean
      << ",stdev=" << stdev << "]";
}

}  // namespace data
}  // namespace mir

