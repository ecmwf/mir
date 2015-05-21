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

namespace mir {
namespace caching {

/*
    What's left todo from Baudouin's code review:

4 - Writting is done with std::fstream which does not throw exception unless asked explicitally
    like so: ofs.exceptions ( std::ofstream::failbit | std::ofstream::badbit );
    this means that no error checking is done on writting, this will lead the truncated files
    when file systems become full

    --> class atlas::MeshCache must use DataHandle() to write and read

8 - File names do not take compiler, number of bits, architecture, etc... in account.
    In certain cases we will have to put these files in shared filesytems (e.g. $SCRATCH).
b - Code should ASSERT() that what their are decoding looks correct. This can be done by sticking a header
    in front of the files, and cheching that the decoder can understand the header correctly.

    --> We'll use the eckitCacheDir to create caches that have the compiler and architecture in them
        However, TODO: stick a header with versions & platform info (compiler, architecture, etc)

*/

// using eckit::CacheManager;
// using eckit::Log;
// using eckit::Resource;
// using eckit::FileHandle;
// using eckit::AutoClose;
// using eckit::PathName;

WeightCache::WeightCache( bool nocache ) : CacheManager("mir/weights"), nocache_(nocache) {
}

const char* WeightCache::version() const {
    return mir_version_str();
}
const char* WeightCache::extension() const {
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

    if(nocache_) return;

    typedef method::WeightMatrix::Index Index;

    eckit::PathName tmp = stage(key);

    eckit::Log::info() << "Inserting weights in cache : " << tmp << "" << std::endl;

    eckit::Timer timer("Saving weights to cache");

    {
        eckit::BufferedHandle f(tmp.fileHandle());

        f.openForWrite(0);
        eckit::AutoClose closer(f);

        // write nominal size of matrix

        Index innerSize = W.innerSize();
        Index outerSize = W.outerSize();

        f.write(&innerSize, sizeof(innerSize));
        f.write(&outerSize, sizeof(outerSize));

        // find all the non-zero values (aka triplets)

        std::vector<Eigen::Triplet<double> > trips;
        for (size_t i = 0; i < W.outerSize(); ++i) {
            for (method::WeightMatrix::InnerIterator it(W, i); it; ++it) {
                trips.push_back(Eigen::Triplet<double>(it.row(), it.col(), it.value()));
            }
        }

        // save the number of triplets

        Index ntrips = trips.size();
        f.write(&ntrips, sizeof(ntrips));

        // now save the triplets themselves

        for (size_t i = 0; i < trips.size(); i++) {

            Eigen::Triplet<double> &rt = trips[i];

            Index x = rt.row();
            Index y = rt.col();
            double w = rt.value();

            f.write(&x, sizeof(x));
            f.write(&y, sizeof(y));
            f.write(&w, sizeof(w));
        }
    }

    ASSERT(commit(key, tmp));
}

bool WeightCache::retrieve(const std::string &key, method::WeightMatrix &W) const {

    if(nocache_) return false;

    typedef method::WeightMatrix::Index Index;

    eckit::PathName path;

    if (!get(key, path))
        return false;

    eckit::Log::info() << "Found weights in cache : " << path << "" << std::endl;
    eckit::Timer timer("Loading weights from cache");

    {
        eckit::BufferedHandle f(path.fileHandle());

        f.openForRead();
        eckit::AutoClose closer(f);

        // read inpts, outpts sizes of matrix

        Index inner, outer;

        f.read(&inner, sizeof(inner));
        f.read(&outer, sizeof(outer));

        Index npts;
        f.read(&npts, sizeof(npts));

        // read total sparse points of matrix (so we can reserve)

        std::vector<Eigen::Triplet<double> > insertions;

        eckit::Log::info() << "Inner: " << eckit::BigNum(inner)
                           << ", outer: " << eckit::BigNum(outer)
                           << ", number of points: " << eckit::BigNum(npts) << std::endl;

        insertions.reserve(npts);

        // read the values

        for (size_t i = 0; i < npts; i++) {
            Index x, y;
            double w;
            f.read(&x, sizeof(x));
            f.read(&y, sizeof(y));
            f.read(&w, sizeof(w));
            insertions.push_back(Eigen::Triplet<double>(x, y, w));
        }

        // check matrix is correctly sized
        // note that Weigths::Matrix is row-major, so rows are outer size

        ASSERT(W.rows() == outer);
        ASSERT(W.cols() == inner);

        // set the weights from the triplets

        double now = timer.elapsed();
        W.setFromTriplets(insertions.begin(), insertions.end());
        eckit::Log::info() << "Inserting " << eckit::Plural(insertions.size(), "triplet") << " in " << eckit::Seconds(timer.elapsed() - now) << std::endl;
    }

    return true;
}


}  // namespace method
}  // namespace mir

