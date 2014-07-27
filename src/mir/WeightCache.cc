/*
 * (C) Copyright 1996-2014 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include <string>
#include <sstream>
#include <iostream>
#include <fstream>

#include "eckit/log/Log.h"
#include "eckit/thread/AutoLock.h"
#include "eckit/io/FileHandle.h"
#include "eckit/config/Resource.h"
#include "eckit/maths/Eigen.h" // always include Eigen via eckit

#include "mir/WeightCache.h"

using namespace eckit;

namespace mir {

//------------------------------------------------------------------------------------------------------

LocalPathName WeightCache::filename(const std::string& key)
{
    PathName base_path = Resource<PathName>("$MIR_CACHE_DIR;MirCacheDir","/tmp/cache/mir");

    PathName f = base_path / "weights" / PathName( key + ".cache" );

    return f.asString();
}

bool WeightCache::add(const std::string& key, Weights::Matrix& W )
{
    LocalPathName file( filename(key) );

    if( file.exists() )
    {
        Log::debug() << "WeightCache entry " << file << " already exists..." << std::endl;
        return false;
    }

    file.dirName().mkdir(); // ensure directory exists

    // unique file name avoids race conditions on the file from multiple processes

    LocalPathName tmpfile ( LocalPathName::unique(file) );

    Log::info() << "inserting weights in cache (" << file << ")" << std::endl;

    std::ofstream ofs;
    ofs.open( tmpfile.c_str(), std::ios::binary );
    
    // write nominal size of matrix

    long innerSize = W.innerSize();
    long outerSize = W.outerSize();
    
    ofs.write(reinterpret_cast<const char*>(&innerSize), sizeof(innerSize));
    ofs.write(reinterpret_cast<const char*>(&outerSize), sizeof(outerSize));
    
    // find all the non-zero values (aka triplets) 

    std::vector<Eigen::Triplet<double> > trips;
    for (unsigned int i = 0; i < W.outerSize(); ++i) 
    {
        for (typename Weights::Matrix::InnerIterator it(W,i); it; ++it)
        {
            trips.push_back(Eigen::Triplet<double>(it.row(), it.col(), it.value()));
        }
    }    
    
    // save the number of triplets 

    long ntrips = trips.size();
    ofs.write(reinterpret_cast<const char*>(&ntrips), sizeof(ntrips));
    
    // now save the triplets themselves

    for (unsigned int i = 0; i < trips.size(); i++)
    {
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

    try
    {
        LocalPathName::rename( tmpfile, file );
    }
    catch( FailedSystemCall& e ) // ignore failed system call -- another process nay have created the file meanwhile
    {
        Log::debug() << "Failed rename of cache file -- " << e.what() << std::endl;
    }

    return true;
}

bool WeightCache::get(const std::string& key, Weights::Matrix& W )
{
    LocalPathName file( filename(key) );

    if( ! file.exists() )
    {
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

    for (unsigned int i = 0; i < npts; i++)
    {
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

//------------------------------------------------------------------------------------------------------

} // namespace mir
