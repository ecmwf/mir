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


#ifndef mir_util_Trace_h
#define mir_util_Trace_h

#include "eckit/log/ResourceUsage.h"
#include "eckit/log/Timer.h"

#include "mir/util/Log.h"


namespace mir {
namespace trace {


namespace detail {


template <typename T>
struct TraceT {
    explicit TraceT(const std::string& name, Log::Channel& log = Log::debug()) { info_ = new T(name.c_str(), log); }
    ~TraceT() { delete info_; }
    double elapsed() { return info_->elapsed(); }

private:
    TraceT(const TraceT&) = delete;
    TraceT& operator=(const TraceT&) = delete;

    T* info_ = nullptr;
};


struct TimerAndResourceUsage : eckit::Timer, eckit::ResourceUsage {
    TimerAndResourceUsage(const std::string& name, Log::Channel& log = Log::debug()) :
        Timer(name, log), ResourceUsage(name, log) {}
};


}  // namespace detail


using ResourceUsage = detail::TraceT<eckit::ResourceUsage>;
using Trace         = detail::TraceT<detail::TimerAndResourceUsage>;
using eckit::Timer;


struct ProgressTimer : public Timer {
    using Plural = Log::Plural;

    /// @param name of the timer
    /// @param limit counter maximum value
    /// @param units unit/units
    /// @param time how often to output progress, based on elapsed time
    /// @param o output stream
    ProgressTimer(const std::string& name, size_t limit, const Plural& units, Log::Channel& = Log::info(),
                  double time = 5.);
    bool operator++();

private:
    double lastTime_;
    size_t counter_;

    const Plural units_;
    const size_t limit_;
    const double time_;
};


}  // namespace trace
}  // namespace mir


#endif
