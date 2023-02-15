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
