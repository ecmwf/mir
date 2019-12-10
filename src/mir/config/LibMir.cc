/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Tiago Quintino
/// @author Pedro Maciel
/// @date August 2016


#include "mir/config/LibMir.h"

#include "mir/api/mir_version.h"

#include "eckit/config/Resource.h"
#include "eckit/filesystem/PathName.h"


namespace mir {


REGISTER_LIBRARY(LibMir);


LibMir::LibMir() : Library("mir") {
}


std::string LibMir::cacheDir() {
    static std::string mirCachePath = eckit::PathName(
                eckit::LibResource<eckit::PathName, LibMir>(
                    "mir-cache-path;"
                    "$MIR_CACHE_PATH",
                    "/tmp/cache" ));
    return mirCachePath;
}


bool LibMir::caching() {
    static bool mirCaching = eckit::LibResource<bool, LibMir>(
                "mir-caching;"
                "$MIR_CACHING",
                true );
    return mirCaching;
}


const LibMir::keywords_t LibMir::postProcess() {
    static const keywords_t defaultKeywords {
        "accuracy",
        "bitmap",
        "checkerboard",
        "compatibility",
        "edition",
        "filter",
        "format",
        "formula",
        "frame",
        "griddef",
        "latitudes",
        "longitudes",
        "packing",
        "pattern",
        "vod2uv",
    };

    auto& config = instance().configuration();
    static const keywords_t keywords = config.getStringVector("post-process", defaultKeywords);

    return keywords;
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

    return sha1.substr(0, std::min(count, 40u));
}


} // namespace mir
