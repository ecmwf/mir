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


#pragma once

#include <string>

#include "eckit/filesystem/PathName.h"
#include "eckit/system/Library.h"


namespace mir {


class LibMir : public eckit::system::Library {
public:
    static const LibMir& instance();

    static std::string homeDir();
    static std::string cacheDir();

    static bool caching();
    static const std::string& lsmNamed();

    enum config_file
    {
        AREA = 0,
        CLASSES,
        GRIB_INPUT,
        GRIB_OUTPUT,
        GRIDS,
        NETCDF,
        PARAMETER_CLASS,
        PARAMETERS,
        ALL_CONFIG_FILES
    };

    static eckit::PathName configFile(config_file);

    enum cache_loader
    {
        LEGENDRE,
        MATRIX,
        POINT_SEARCH,
        ALL_CACHE_LOADERS,
    };

    static std::string cacheLoader(cache_loader);

private:
    LibMir();

    const void* addr() const override;
    std::string version() const override;
    std::string gitsha1(unsigned int count) const override;
};


}  // namespace mir
