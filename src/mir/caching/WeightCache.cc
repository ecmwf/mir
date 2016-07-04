/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "WeightCache.h"

#include "eckit/io/BufferedHandle.h"
#include "eckit/log/Timer.h"
#include "eckit/log/Plural.h"
#include "eckit/log/BigNum.h"

#include "eckit/log/Seconds.h"
#include "mir/api/mir_version.h"
#include "mir/log/MIR.h"

namespace mir {
namespace caching {

/*
    What's left todo from Baudouin's code review:

4 - Writting is done with std::fstream which does not throw exception unless asked explicitally
    like so: ofs.exceptions ( std::ofstream::failbit | std::ofstream::badbit );
    this means that no error checking is done on writing, this will lead the truncated files
    when file systems become full

    --> class atlas::mesh::MeshCache must use DataHandle() to write and read

8 - File names do not take compiler, number of bits, architecture, etc... in account.
    In certain cases we will have to put these files in shared filesytems (e.g. $SCRATCH).
b - Code should ASSERT() that what their are decoding looks correct. This can be done by sticking a header
    in front of the files, and cheching that the decoder can understand the header correctly.

    --> We'll use the eckitCacheDir to create caches that have the compiler and architecture in them
        However, TODO: stick a header with versions & platform info (compiler, architecture, etc)

*/


WeightCache::WeightCache():
    CacheManager("mir/weights") {
}

const char *WeightCache::version() const {
    return "1"; // Change me if the cache file structure changes
}

const char *WeightCache::extension() const {
    return ".mat";
}

void WeightCache::print(std::ostream &s) const {
    s << "WeightCache[";
    CacheManager::print(s);
    s << "name=" << name() << ","
      << "version=" << version() << ","
      << "extention=" << extension() << ","
      << "]";
}

void WeightCache::insert(const std::string &key, const method::WeightMatrix &W) const {

    eckit::PathName tmp = stage(key);

    eckit::Log::info() << "Inserting weights in cache : " << tmp << "" << eckit::newl;

    // eckit::TraceTimer<MIR> timer("Saving weights to cache");
    W.save(tmp);

    ASSERT(commit(key, tmp));
}

bool WeightCache::retrieve(const std::string &key, method::WeightMatrix &W) const {

    eckit::PathName path;

    if (!get(key, path))
        return false;

    // eckit::Log::info() << "Found weights in cache : " << path << "" << eckit::newl;
    // eckit::TraceTimer<MIR> timer("Loading weights from cache");

    W.load(path);

    return true;
}


}  // namespace method
}  // namespace mir

