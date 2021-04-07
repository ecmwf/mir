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

#include <map>
#include <string>


namespace mir {
namespace netcdf {
class NCFile;
}
}  // namespace mir


namespace mir {
namespace netcdf {


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


}  // namespace netcdf
}  // namespace mir
