// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/config/LibMir.h"

#include <algorithm>
#include <set>
#include <sstream>
#include <vector>

#include "eckit/cache/DiskCache.h"
#include "eckit/cache/Roots.h"
#include "eckit/config/Configuration.h"
#include "eckit/config/Resource.h"
#include "eckit/filesystem/PathName.h"
#include "eckit/log/JSON.h"
#include "eckit/utils/MD5.h"

#include "mir/api/mir_version.h"
#include "mir/caching/InMemoryCacheBase.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"


namespace mir {


REGISTER_LIBRARY(LibMir);


LibMir::LibMir() : Library("mir") {}


std::string LibMir::cacheDir() {
    static std::string mirCachePath =
        eckit::PathName(eckit::LibResource<eckit::PathName, LibMir>("mir-cache-path;$MIR_CACHE_PATH", "/tmp/cache"));
    return mirCachePath;
}


namespace {


/// Cache paths, as the caches use them (read in order, written in the first writable)
eckit::cache::Roots cache_roots() {
    return eckit::cache::Roots{LibMir::cacheDir()};
}


}  // namespace


std::string LibMir::cacheInfo() {
    const auto roots    = cache_roots();
    const auto writable = roots.writablePaths();

    std::ostringstream out;
    eckit::JSON j(out);

    j.startObject();

    j << "memory";
    caching::InMemoryCacheBase::json(j);

    j << "disk";
    j.startObject();

    j << "control";
    j.startObject();
    j << "caching" << caching();
    j << "throw_on_miss" << cacheThrowOnMiss();
    j << "paths";
    j.startObject();
    j << "readable";
    j.startList();
    for (const auto& p : roots.readablePaths()) {
        j << p.asString();
    }
    j.endList();
    j << "writable";
    j.startList();
    for (const auto& p : writable) {
        j << p.asString();
    }
    j.endList();
    j.endObject();
    j.endObject();

    // cache files, where they are written (the first writable path), by directory
    if (!writable.empty() && (writable.front() / "mir").exists()) {
        std::vector<eckit::PathName> files;
        std::vector<eckit::PathName> dirs;
        (writable.front() / "mir").children(files, dirs);

        for (const auto& d : dirs) {
            j << d.baseName();
            j.startObject();
            j << "size" << eckit::cache::DiskCache::size(d);
            j.endObject();
        }
    }
    j.endObject();

    j.endObject();
    return out.str();
}


void LibMir::purgeCache() {
    caching::InMemoryCacheBase::purgeAll();
}


void LibMir::removeCacheDir() {
    if (const auto writable = cache_roots().writablePaths(); !writable.empty()) {
        eckit::cache::DiskCache::remove(writable.front() / "mir");
    }
}


std::string LibMir::homeUrl() {
    static std::string mirHomeUrl =
        eckit::LibResource<std::string, LibMir>("mir-home-url;$MIR_HOME_URL", "https://sites.ecmwf.int/repository/mir");
    return mirHomeUrl;
}


bool LibMir::caching() {
    static bool mirCaching = eckit::LibResource<bool, LibMir>("mir-caching;$MIR_CACHING", true);
    return mirCaching;
}


bool LibMir::cacheThrowOnMiss() {
    static const bool yes = eckit::Resource<bool>("$MIR_THROW_ON_CACHE_MISS;mirThrowOnCacheMiss", false);
    return yes;
}


const std::string& LibMir::lsmNamed() {
    static const std::string mirLsmNamed =
        eckit::LibResource<std::string, LibMir>("mir-lsm-named;$MIR_LSM_NAMED", "1km.climate.v020");
    return mirLsmNamed;
}


const std::string& LibMir::gribSampleName() {
    // FIXME: make this edition-independent
    static const std::string mirGribSample{[&](std::string value) {
        instance().configuration().get("grib-sample", value);
        return eckit::LibResource<std::string, LibMir>("mir-grib-sample;$MIR_GRIB_SAMPLE", value);
    }("GRIB2")};
    return mirGribSample;
}


eckit::PathName LibMir::configFile(config_file c) {
    using r = eckit::LibResource<std::string, LibMir>;

    static const eckit::PathName files[]{
        {r("mir-config-area;$MIR_CONFIG_AREA", "~mir/etc/mir/area.yaml")},
        {r("mir-config-classes;$MIR_CONFIG_CLASSES", "~mir/etc/mir/classes.yaml")},
        {r("mir-config-grib-input;$MIR_CONFIG_GRIB_INPUT", "~mir/etc/mir/grib-input.yaml")},
        {r("mir-config-grib-output;$MIR_CONFIG_GRIB_OUTPUT", "~mir/etc/mir/grib-output.yaml")},
        {r("mir-config-grid-type;$MIR_CONFIG_GRID_TYPE", "~mir/etc/mir/grid-type.yaml")},
        {r("mir-config-grids;$MIR_CONFIG_GRIDS", "~mir/etc/mir/grids.yaml")},
        {r("mir-config-limiter;$MIR_CONFIG_LIMITER", "~mir/etc/mir/limiter.yaml")},
        {r("mir-config-lsm;$MIR_CONFIG_LSM", "~mir/etc/mir/lsm.yaml")},
        {r("mir-config-netcdf;$MIR_CONFIG_NETCDF", "~mir/etc/mir/netcdf.yaml")},
        {r("mir-config-parameter-class;$MIR_CONFIG_PARAMETER_CLASS", "~mir/etc/mir/parameter-class.yaml")},
        {r("mir-config-parameters;$MIR_CONFIG_PARAMETERS", "~mir/etc/mir/parameters.yaml")},
        {r("mir-config-style;$MIR_CONFIG_STYLE", "~mir/etc/mir/style.yaml")},
    };

    ASSERT(0 <= c && c < config_file::ALL_CONFIG_FILES);
    const auto& path = files[c];

    if (!path.exists()) {
        const std::string msg =
            "Configuration file '" + path.fullName() + "' not found, post-processing defaults might not be appropriate";

        static bool abort = eckit::Resource<bool>("$MIR_ABORT_IF_CONFIGURATION_NOT_FOUND", true);
        if (abort) {
            Log::error() << msg << std::endl;
            throw exception::UserError(msg);
        }

        // only log messages once
        static std::set<eckit::Hash::digest_t> known_messages;
        if (known_messages.insert(eckit::MD5(msg).digest()).second) {
            Log::warning() << msg << std::endl;
        }
    }

    return path;
}


std::string LibMir::cacheLoader(cache_loader l) {
    using r = eckit::LibResource<std::string, LibMir>;

    static const std::string loaders[]{
        {r("$MIR_LEGENDRE_LOADER;mirLegendreLoader", "mapped-memory")},
        {r("$MIR_MATRIX_LOADER;mirMatrixLoader", "file-io")},
        {r("$MIR_POINT_SEARCH_LOADER;mirPointSearchLoader", "mapped-cache-file")},
    };

    ASSERT(0 <= l && l < cache_loader::ALL_CACHE_LOADERS);
    return loaders[l];
}


const std::vector<std::string>& LibMir::postProcessKeys() {
    static const std::vector<std::string> defaults{
        "accuracy", "bitmap",  "checkerboard", "compatibility", "edition", "filter",  "format",  "formula",
        "frame",    "griddef", "latitudes",    "longitudes",    "nabla",   "packing", "pattern", "vod2uv",
    };

    static const auto& keys = instance().configuration().getStringVector("post-process", defaults);
    return keys;
}


const LibMir& LibMir::instance() {
    static LibMir libmir;
    return libmir;
}


std::string LibMir::homeDir() {
    return LibMir::instance().libraryHome();
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
