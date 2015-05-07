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

#include <sys/types.h>
#include <sys/stat.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "eckit/config/Resource.h"
#include "eckit/io/FileHandle.h"
#include "eckit/io/BufferedHandle.h"

#include "eckit/log/Timer.h"
#include "eckit/log/Plural.h"
#include "eckit/log/Seconds.h"

namespace mir {
namespace method {

/*
I was asked to review that code, so there it is:

1 - We discussed having a generic caching class in eckit, from which we derive, this is not the case.
    There is also a need to cache legendre coefficients, and we want code reuse.
2 - The code does not ensure that the created directories (mkdir) are world readable/writable/executable
    so that other users can access them and create new files in them (umask? chmod?)
3 - The code does not ensure that the created files (*.cache) are world readable
    so that other users can access them (umask?)
4 - Writting is done with std::fstream which does not throw exception unless asked explicitally
    like so: ofs.exceptions ( std::ofstream::failbit | std::ofstream::badbit );
    this means that no error checking is done on writting, this will lead the truncated files
    when file systems become full
5 - Reading and writting code are not symetrical (write uses std::fstream, read uses DataHandle)
6 - One should AutoClose<FileHandle> to ensure that handles are closed in case of
    exception (same for std::ftream), so we don't leak file descriptors
7 - DataHandle reads into a void*, not a char*, so there is not need for reinterpret_cast<const char*>
8 - File names do not take compiler, number of bits, architecture, etc... in account. In certain cases
    we will have to put these files in shared filesytems (e.g. $SCRATCH).
9 - There is no check done that the code used to read the file matched the code used to right the file.
    Old and new version of MIR (i.e. of MARS clients) will coexists. This should be refelected in the file
    name, e.g. adding a version number of the encoding code (no need for fancy SHA1, just 1, 2, 3...)
10- SparseMatrixBase<T>::innerSize() return Index, not long. Code should reflect that (consider point 9).
    This applies to many other variables.
11- Variable ntrips should be size_t
12- Use size_t instead of unsigned int
13- Use camelCase: filename() => fileName()
14- Log() message should be English sentences, and thus start with a capital letter

The points 8 and 9 could be addressed externally, i.e. having different cache directories
for different architecture, compilers, etc, but:

a - Doing so will make it more difficult to share cache files between different applications (MARS, mir tool,
    PRODGEN, ...)
b - Code should ASSERT() that what their are decoding looks correct. This can be done by sticking a header
    in front of the files, and cheching that the decoder can understand the header correctly.

Point 12-13 are stylistic but important if you are serious about a common code base.

TODO:
1 - Create a generic file caching class in eckit that takes care of management of directories and files,
    ensuring proper permission settings.
2 - This class provide DataHandles to its subclasses to read/write their stuff
3 - Code must ensure that no resource are lost in case of exception  (e.g. using AutoClose<T>)
*/

using eckit::CacheManager;
using eckit::Log;
using eckit::Resource;
using eckit::FileHandle;
using eckit::AutoClose;
using eckit::PathName;
using atlas::Grid;

WeightCache::WeightCache() : CacheManager("weights") {
}

PathName WeightCache::entry(const key_t &key) const {
    PathName base_path = Resource<PathName>("$MIR_CACHE_DIR;MirCacheDir", "/tmp/cache/mir");
    PathName f = base_path / name() / PathName( key + ".cache" );
    return f;
}

std::string WeightCache::generateKey(const std::string &method,
                                     const atlas::Grid &in,
                                     const atlas::Grid &out,
                                     const lsm::LandSeaMask &maskin,
                                     const lsm::LandSeaMask &maskout) const {
    std::ostringstream s;
    s << method << "." << in.unique_id() << "." << out.unique_id();
    if (maskin.active()) {
        s << ".IM" << maskin.unique_id();
    }
    if (maskout.active()) {
        s << ".OM" << maskout.unique_id();
    }
    return s.str();
}

void WeightCache::insert(const std::string &key, const WeightMatrix &W) {

    PathName tmp_path = stage(key);

    Log::info() << "Inserting weights in cache (" << tmp_path << ")" << std::endl;

    eckit::Timer timer("Saving weights to cache");

    {
        eckit::BufferedHandle f(tmp_path.fileHandle());

        f.openForWrite(0); AutoClose closer(f);

        // write nominal size of matrix

        long innerSize = W.innerSize();
        long outerSize = W.outerSize();

        f.write(&innerSize, sizeof(innerSize));
        f.write(&outerSize, sizeof(outerSize));

        // find all the non-zero values (aka triplets)

        std::vector<Eigen::Triplet<double> > trips;
        for (unsigned int i = 0; i < W.outerSize(); ++i) {
            for (WeightMatrix::InnerIterator it(W, i); it; ++it) {
                trips.push_back(Eigen::Triplet<double>(it.row(), it.col(), it.value()));
            }
        }

        // save the number of triplets

        long ntrips = trips.size();
        f.write(&ntrips, sizeof(ntrips));

        // now save the triplets themselves

        for (unsigned int i = 0; i < trips.size(); i++) {

            Eigen::Triplet<double> &rt = trips[i];

            long x = rt.row();
            long y = rt.col();
            double w = rt.value();

            f.write(&x, sizeof(x));
            f.write(&y, sizeof(y));
            f.write(&w, sizeof(w));
        }
    }

    commit(key, tmp_path);
}

bool WeightCache::retrieve(const std::string &key, WeightMatrix &W) const {

    PathName path;

    if (!get(key, path))
        return false;

    Log::info() << "Found weights in cache (" << path << ")" << std::endl;
    eckit::Timer timer("Loading weights from cache");

    {
        eckit::BufferedHandle f(path.fileHandle());

        f.openForRead(); AutoClose closer(f);

        // read inpts, outpts sizes of matrix

        long inner, outer;

        f.read(&inner, sizeof(inner));
        f.read(&outer, sizeof(outer));

        long npts;
        f.read(&npts, sizeof(npts));

        // read total sparse points of matrix (so we can reserve)

        std::vector<Eigen::Triplet<double> > insertions;

        insertions.reserve(npts);

        // read the values

        for (unsigned int i = 0; i < npts; i++) {
            long x, y;
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

