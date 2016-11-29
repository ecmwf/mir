/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "mir/caching/WeightCache.h"
#include "mir/config/LibMir.h"
#include "mir/method/WeightMatrix.h"

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
    CacheManager(LibMir::cacheDir(),
                 eckit::Resource<bool>("$MIR_THROW_ON_CACHE_MISS;mirThrowOnCacheMiss",
                                       false)) {
}

const char *WeightCacheTraits::name() {
    return "mir/weights";
}

int WeightCacheTraits::version() {
    return 2;
}

const char *WeightCacheTraits::extension() {
    return ".mat";
}

void WeightCacheTraits::save(const value_type& W, const eckit::PathName& path) {
    eckit::Log::info() << "Inserting weights in cache : " << path << "" << std::endl;

    eckit::TraceTimer<LibMir> timer("Saving weights to cache");
    W.save(path);
}

void WeightCacheTraits::load(value_type& W, const eckit::PathName& path) {
    eckit::TraceTimer<LibMir> timer("Loading weights from cache");

    W.load(path);
    W.validate("fromCache");
}


}  // namespace method
}  // namespace mir

