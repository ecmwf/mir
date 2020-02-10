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


#ifndef mir_util_TraceResourceUsage_h
#define mir_util_TraceResourceUsage_h

namespace eckit {
class ResourceUsage;
}

namespace mir {
namespace util {

class TraceResourceUsage {
public:
    explicit TraceResourceUsage(const char* name);
    ~TraceResourceUsage();

private:
    TraceResourceUsage(const TraceResourceUsage&) = delete;
    TraceResourceUsage& operator=(const TraceResourceUsage&) = delete;

    eckit::ResourceUsage* info_ = nullptr;
};

}  // namespace util
}  // namespace mir


#endif
