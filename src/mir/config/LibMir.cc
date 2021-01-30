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


#include "mir/config/LibMir.h"

#include "mir/api/mir_version.h"

#include "eckit/config/Resource.h"
#include "eckit/filesystem/PathName.h"


namespace mir {


REGISTER_LIBRARY(LibMir);


LibMir::LibMir() : Library("mir") {}


std::string LibMir::cacheDir() {
    static std::string mirCachePath =
        eckit::PathName(eckit::LibResource<eckit::PathName, LibMir>("mir-cache-path;"
                                                                    "$MIR_CACHE_PATH",
                                                                    "/tmp/cache"));
    return mirCachePath;
}


bool LibMir::caching() {
    static bool mirCaching = eckit::LibResource<bool, LibMir>(
        "mir-caching;"
        "$MIR_CACHING",
        true);
    return mirCaching;
}


const LibMir& LibMir::instance() {
    static LibMir libmir;
    return libmir;
}


const void* LibMir::addr() const {
    return this;
}


std::string LibMir::version() const {
    return mir_version_str();
}


std::string LibMir::gitsha1(unsigned int count) const {
    std::string sha1(mir_git_sha1());
    if (sha1.empty()) {
        return "not available";
    }

    constexpr unsigned int MAX_LENGTH = 40U;
    return sha1.substr(0, std::min(count, MAX_LENGTH));
}


}  // namespace mir
