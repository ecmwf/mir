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


#pragma once

#include "eckit/log/Timer.h"

#include "mir/util/Log.h"


namespace eckit {
class ResourceUsage;
}


namespace mir::trace {


class Timer : public eckit::Timer {
public:
    explicit Timer(const std::string&);

    using eckit::Timer::elapsed;
    double elapsed(double t) { return eckit::Timer::elapsed() - t; }
    Log::Seconds elapsedSeconds(double t = 0, bool compact = false) { return {elapsed(t), compact}; }
};


struct ResourceUsage : public Timer {
    explicit ResourceUsage(const std::string&);

    ResourceUsage(const ResourceUsage&) = delete;
    ResourceUsage(ResourceUsage&&)      = delete;

    ~ResourceUsage();

    void operator=(const ResourceUsage&) = delete;
    void operator=(ResourceUsage&&)      = delete;

private:
    eckit::ResourceUsage* info_;
};


struct ProgressTimer : public Timer {

    /// @param name of the timer
    /// @param limit counter maximum value
    /// @param units unit/units
    /// @param time how often to output progress, based on elapsed time
    /// @param out output stream
    ProgressTimer(const std::string& name, size_t limit, const Log::Plural& units, double time = 5.);

    bool operator++();

private:
    double lastTime_;
    size_t counter_;

    const Log::Plural units_;
    const size_t limit_;
    const double time_;
};


}  // namespace mir::trace
