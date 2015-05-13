/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "LegendreCache.h"

// #include <sys/types.h>
// #include <sys/stat.h>

// #include <fstream>
// #include <iostream>
// #include <sstream>
// #include <string>

// #include "eckit/config/Resource.h"
// #include "eckit/io/FileHandle.h"
// #include "eckit/io/BufferedHandle.h"
// #include "eckit/log/Timer.h"
// #include "eckit/log/Plural.h"
// #include "eckit/log/BigNum.h"

// #include "eckit/log/Seconds.h"

// #include "atlas/Grid.h"

// #include "mir/api/mir_version.h"
// #include "mir/lsm/Mask.h"
// #include "mir/method/Method.h"
// #include "mir/method/WeightMatrix.h"

namespace mir {
namespace method {



LegendreCache::LegendreCache() : CacheManager("mir/weights") {
}

const char* LegendreCache::version() const {
    return mir_version_str();
}
const char* LegendreCache::extension() const {
    return ".leg";
}


void LegendreCache::print(std::ostream &s) const {
    s << "LegendreCache[";
    CacheManager::print(s);
    s << "name=" << name() << ","
      << "version=" << version() << ","
      << "extention=" << extension() << ","
      << "]";
}


}  // namespace method
}  // namespace mir

