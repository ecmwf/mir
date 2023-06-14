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

#include <set>

#include "eckit/config/Resource.h"
#include "eckit/filesystem/PathName.h"
#include "eckit/utils/MD5.h"

#include "mir/api/mir_version.h"
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


bool LibMir::caching() {
    static bool mirCaching = eckit::LibResource<bool, LibMir>("mir-caching;$MIR_CACHING", true);
    return mirCaching;
}


eckit::PathName LibMir::configFile(config_file c) {
    using r = eckit::LibResource<std::string, LibMir>;

    static const eckit::PathName files[]{
        {r("mir-config-area;$MIR_CONFIG_AREA", "~mir/etc/mir/area.yaml")},
        {r("mir-config-classes;$MIR_CONFIG_CLASSES", "~mir/etc/mir/classes.yaml")},
        {r("mir-config-grib-input;$MIR_CONFIG_GRIB_INPUT", "~mir/etc/mir/grib-input.yaml")},
        {r("mir-config-grib-output;$MIR_CONFIG_GRIB_OUTPUT", "~mir/etc/mir/grib-output.yaml")},
        {r("mir-config-grids;$MIR_CONFIG_GRIDS", "~mir/etc/mir/grids.yaml")},
        {r("mir-config-netcdf;$MIR_CONFIG_NETCDF", "~mir/etc/mir/netcdf.yaml")},
        {r("mir-config-parameter-class;$MIR_CONFIG_PARAMETER_CLASS", "~mir/etc/mir/parameter-class.yaml")},
        {r("mir-config-parameters;$MIR_CONFIG_PARAMETERS", "~mir/etc/mir/parameters.yaml")},
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
