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


#include "mir/util/Trace.h"

#include "eckit/config/Resource.h"
#include "eckit/log/ETA.h"
#include "eckit/log/ResourceUsage.h"

#include "mir/config/LibMir.h"
#include "mir/util/Log.h"


namespace mir {
namespace trace {


ResourceUsage::ResourceUsage(const char* name, Log::Channel& log) : Timer(name, log) {
    static bool usage = eckit::LibResource<bool, LibMir>(
        "mir-trace-resource-usage;"
        "$MIR_TRACE_RESOURCE_USAGE",
        false);
    info_ = usage ? new eckit::ResourceUsage(name, log) : nullptr;
}


ResourceUsage::~ResourceUsage() {
    delete info_;
}


ProgressTimer::ProgressTimer(const std::string& name, size_t limit, const Log::Plural& units, Log::Channel& out,
                             double time) :
    Timer(name, out), lastTime_(0.), counter_(0), units_(units), limit_(limit), time_(time) {}


bool ProgressTimer::operator++() {
    bool hasOutput = (0 < counter_) && (lastTime_ + time_ < elapsed());

    if (hasOutput) {
        lastTime_   = elapsed();
        double rate = double(counter_) / lastTime_;
        output() << Log::Pretty(counter_, units_) << " in " << Log::Seconds(lastTime_) << ", rate: " << rate << " "
                 << units_(counter_) << "/s"
                 << ", ETA: " << eckit::ETA(double(limit_ - counter_) / rate) << std::endl;
    }

    if (counter_ < limit_) {
        ++counter_;
    }

    return hasOutput;
}


}  // namespace trace
}  // namespace mir
