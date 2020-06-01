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


#include "mir/util/Pretty.h"

#include <iostream>

#include "eckit/log/BigNum.h"
#include "eckit/log/ETA.h"
#include "eckit/log/Seconds.h"


namespace mir {


static const Pretty::Plural noPlural("", "");


Pretty::Pretty(int count) : Pretty(count, noPlural) {}


Pretty::Pretty(size_t count) : Pretty(count, noPlural) {}


void Pretty::print(std::ostream& s) const {
    s << eckit::BigNum(count_);
    if (plural_) {
        s << ' ' << plural_(count_);
    }
}


Pretty::PrettyProgress::PrettyProgress(const std::string& name, size_t limit, const Pretty::Plural& units,
                                       std::ostream& o) :
    Timer(name, o), lastTime_(0.), counter_(0), units_(units), limit_(limit) {}


bool Pretty::PrettyProgress::operator++() {
    bool out = hasOutput();

    if (out) {
        lastTime_   = elapsed();
        double rate = counter_ / lastTime_;
        output() << Pretty(counter_, units_) << " in " << eckit::Seconds(lastTime_) << ", rate: " << rate << " "
                 << units_(counter_) << "/s"
                 << ", ETA: " << eckit::ETA((limit_ - counter_) / rate) << std::endl;
    }

    if (counter_ < limit_) {
        ++counter_;
    }

    return out;
}


Pretty::ProgressTimer::ProgressTimer(const std::string& name, size_t limit, const Pretty::Plural& units,
                                     std::ostream& o, double time) :
    PrettyProgress(name, limit, units, o), time_(time) {}


bool Pretty::ProgressTimer::hasOutput() {
    return (0 < counter_) && (lastTime_ + time_ < elapsed());
}


Pretty::ProgressCounter::ProgressCounter(const std::string& name, size_t limit, const Pretty::Plural& units,
                                         std::ostream& o, size_t count) :
    PrettyProgress(name, limit, units, o), count_(count) {}


bool Pretty::ProgressCounter::hasOutput() {
    return (0 < counter_) && (counter_ % count_ == 0);
}


}  // namespace mir
