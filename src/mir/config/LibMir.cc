/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Tiago Quintino
/// @date   August 2016

#include "mir/config/LibMir.h"

#include "eckit/config/Resource.h"
#include "eckit/thread/AutoLock.h"
#include "eckit/thread/Mutex.h"
#include "eckit/thread/Once.h"

#include "mir/api/mir_version.h"

using namespace eckit;

namespace mir {

//----------------------------------------------------------------------------------------------------------------------

static LibMir libmir;

LibMir::LibMir() : Library("mir") {}

eckit::PathName LibMir::cacheDir()
{
    static eckit::PathName mirCachePath = Resource<PathName>("mirCachePath;$MIR_CACHE_PATH", "/tmp/cache");
    return mirCachePath;
}

const LibMir& LibMir::instance()
{
    return libmir;
}

const void* LibMir::addr() const { return this; }

std::string LibMir::version() const {
    return mir_version_str();
}

std::string LibMir::gitsha1(unsigned int count) const {
    std::string sha1(mir_git_sha1());
    if (sha1.empty()) {
        return "not available";
    }

    return sha1.substr(0, std::min(count, 40u));
}

//----------------------------------------------------------------------------------------------------------------------

} // namespace mir

