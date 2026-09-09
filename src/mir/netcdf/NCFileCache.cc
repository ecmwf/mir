// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/netcdf/NCFileCache.h"

#include "mir/netcdf/NCFile.h"


namespace mir::netcdf {


NCFileCache::NCFileCache() = default;


NCFileCache::~NCFileCache() {
    for (auto& j : files_) {
        delete j.second;
    }
}


NCFile& NCFileCache::lookUp(const std::string& path) {
    auto j = files_.find(path);
    if (j == files_.end()) {
        return *(files_[path] = new NCFile(path));
    }
    return *(j->second);
}


}  // namespace mir::netcdf
