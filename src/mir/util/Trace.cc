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

#include "mir/config/LibMir.h"
#include "mir/util/Exceptions.h"


namespace mir {
namespace trace {


template <>
detail::TraceT<eckit::ResourceUsage>::TraceT(const std::string& name, Log::Channel& log) {
    static bool usage = eckit::LibResource<bool, LibMir>(
        "mir-trace-resource-usage;"
        "$MIR_TRACE_RESOURCE_USAGE",
        false);
    if (usage) {
        info_ = new eckit::ResourceUsage(name.c_str(), log);
    }
}


template <>
double detail::TraceT<eckit::ResourceUsage>::elapsed() {
    NOTIMP;
}


}  // namespace trace
}  // namespace mir
