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

#include "eckit/config/Resource.h"
#include "eckit/filesystem/PathName.h"

#include "mir/api/mir_version.h"
#include "mir/util/Exceptions.h"


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
        {r("mir-config-grib;$MIR_CONFIG_GRIB", "~mir/etc/mir/GRIB.yaml")},
        {r("mir-config-grids;$MIR_CONFIG_GRIDS", "~mir/etc/mir/grids.yaml")},
        {r("mir-config-netcdf;$MIR_CONFIG_NETCDF", "~mir/etc/mir/netcdf.yaml")},
        {r("mir-config-parameter-class;$MIR_CONFIG_PARAMETER_CLASS", "~mir/etc/mir/parameter-class.yaml")},
        {r("mir-config-parameters;$MIR_CONFIG_PARAMETERS", "~mir/etc/mir/parameters.yaml")},
    };

    ASSERT(0 <= c && c < ALL_CONFIG_FILES);
    return files[c];
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
