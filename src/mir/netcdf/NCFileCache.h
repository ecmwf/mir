// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <map>
#include <string>


namespace mir::netcdf {
class NCFile;
}  // namespace mir::netcdf


namespace mir::netcdf {


class NCFileCache {
public:
    NCFileCache();
    ~NCFileCache();

    NCFile& lookUp(const std::string&);

private:
    NCFileCache(const NCFileCache&);
    NCFileCache& operator=(const NCFileCache&);

    std::map<std::string, NCFile*> files_;
};


}  // namespace mir::netcdf
