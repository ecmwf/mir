/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "soyuz/method/WeightCache.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "eckit/config/Resource.h"
#include "eckit/io/FileHandle.h"


using namespace eckit;


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


PathName WeightCache::filename(const std::string& key) {
    PathName base_path = Resource<PathName>("$MIR_CACHE_DIR;MirCacheDir","/tmp/cache/mir");

    PathName f = base_path / "weights" / PathName( key + ".cache" );

    return f;
}


bool WeightCache::add(const std::string& key, MethodWeighted::Matrix& W ) {
    PathName file( filename(key) );

    if( file.exists() ) {
        Log::debug() << "WeightCache entry " << file << " already exists..." << std::endl;
        return false;
    }

    file.dirName().mkdir(); // ensure directory exists

    // unique file name avoids race conditions on the file from multiple processes

    PathName tmpfile ( PathName::unique(file) );

    Log::info() << "inserting weights in cache (" << file << ")" << std::endl;

    std::ofstream ofs;
    ofs.open( tmpfile.asString().c_str(), std::ios::binary );

    // write nominal size of matrix

    long innerSize = W.innerSize();
    long outerSize = W.outerSize();

    ofs.write(reinterpret_cast<const char*>(&innerSize), sizeof(innerSize));
    ofs.write(reinterpret_cast<const char*>(&outerSize), sizeof(outerSize));

    // find all the non-zero values (aka triplets)

    std::vector<Eigen::Triplet<double> > trips;
    for (unsigned int i = 0; i < W.outerSize(); ++i) {
        for ( MethodWeighted::Matrix::InnerIterator it(W,i); it; ++it) {
            trips.push_back(Eigen::Triplet<double>(it.row(), it.col(), it.value()));
        }
    }

    // save the number of triplets

    long ntrips = trips.size();
    ofs.write(reinterpret_cast<const char*>(&ntrips), sizeof(ntrips));

    // now save the triplets themselves

    for (unsigned int i = 0; i < trips.size(); i++) {
        Eigen::Triplet<double>& rt = trips[i];
        //ofs << (long)rt.row() << (long)rt.col() << (double)rt.value();
        long x = rt.row();
        long y = rt.col();
        double w = rt.value();

        ofs.write(reinterpret_cast<const char*>(&x), sizeof(x));
        ofs.write(reinterpret_cast<const char*>(&y), sizeof(y));
        ofs.write(reinterpret_cast<const char*>(&w), sizeof(w));

    }

    ofs.close();

    // now try to rename the file to its file pathname

    try {
        PathName::rename( tmpfile, file );
    } catch( FailedSystemCall& e ) { // ignore failed system call -- another process nay have created the file meanwhile
        Log::debug() << "Failed rename of cache file -- " << e.what() << std::endl;
    }

    return true;
}


bool WeightCache::get(const std::string& key, MethodWeighted::Matrix& W ) {
    PathName file( filename(key) );

    if( ! file.exists() ) {
        return false;
    }

    Log::info() << "found weights in cache (" << file << ")" << std::endl;

    FileHandle fh( file );

    fh.openForRead();

    // read inpts, outpts sizes of matrix

    long inner, outer;

    fh.read(reinterpret_cast<char*>(&inner), sizeof(inner));
    fh.read(reinterpret_cast<char*>(&outer), sizeof(outer));

    long npts;
    fh.read(reinterpret_cast<char*>(&npts), sizeof(npts));

    // read total sparse points of matrix (so we can reserve)

    std::vector<Eigen::Triplet<double> > insertions;

    insertions.reserve(npts);

    // read the values

    for (unsigned int i = 0; i < npts; i++) {
        long x, y;
        double w;
        //fh >> x >> y >> w;
        fh.read(reinterpret_cast<char*>(&x), sizeof(x));
        fh.read(reinterpret_cast<char*>(&y), sizeof(y));
        fh.read(reinterpret_cast<char*>(&w), sizeof(w));
        insertions.push_back(Eigen::Triplet<double>(x, y, w));
    }

    fh.close();

    // check matrix is correctly sized
    // note that Weigths::Matrix is row-major, so rows are outer size

    ASSERT( W.rows() == outer );
    ASSERT( W.cols() == inner );

    // set the weights from the triplets

    W.setFromTriplets(insertions.begin(), insertions.end());

    return true;
}


}  // namespace method
}  // namespace mir

