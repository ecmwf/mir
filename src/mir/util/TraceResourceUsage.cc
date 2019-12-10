/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/util/TraceResourceUsage.h"

#include "eckit/config/Resource.h"
#include "eckit/log/ResourceUsage.h"

#include "mir/config/LibMir.h"

namespace mir {
namespace util {


TraceResourceUsage::TraceResourceUsage(const char* name) {
    static bool usage = eckit::LibResource<bool, LibMir>(
        "mir-trace-resource-usage;"
        "$MIR_TRACE_RESOURCE_USAGE",
        false);
    if (usage) {
        info_ = new eckit::ResourceUsage(name, eckit::Log::debug<LibMir>());
    }
}

TraceResourceUsage::~TraceResourceUsage() {
    delete info_;
}


}  // namespace util
}  // namespace mir
