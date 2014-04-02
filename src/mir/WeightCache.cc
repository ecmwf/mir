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

#include <Eigen/Sparse>
#include <Eigen/Dense>

#include "eckit/log/Log.h"
#include "eckit/log/Timer.h"
#include "eckit/filesystem/LocalPathName.h"
#include "eckit/thread/AutoLock.h"

#include "atlas/grid/Grid.h"

#include "mir/WeightCache.h"

using eckit::Timer;
using eckit::Mutex;
using eckit::AutoLock;

//-----------------------------------------------------------------------------

namespace mir {

//-----------------------------------------------------------------------------

WeightCache::WeightCache()
{
}

std::string WeightCache::filename(const std::string& key) const
{
    std::stringstream ss;
    ss << "cache/" << key << ".cache";
    return ss.str();
}

bool WeightCache::add(const std::string& key, Eigen::SparseMatrix<double>& W ) const
{
    // @todo need proper locking against multiple threads creating files.
    // @todo use file rename perhaps
    
    AutoLock<Mutex> lock(mutex_);
    
    const std::string fn = filename(key);

    eckit::LocalPathName fpath(fn);
    if(fpath.exists())
        return false;

    std::ofstream ofs;
    ofs.open(fn.c_str(), std::ios::binary);
    
    // write nominal size of matrix
    //ofs << (long)W.innerSize() << (long)W.outerSize();
    //
    long innerSize = W.innerSize();
    long outerSize = W.outerSize();
    
    ofs.write(reinterpret_cast<const char*>(&innerSize), sizeof(innerSize));
    ofs.write(reinterpret_cast<const char*>(&outerSize), sizeof(outerSize));
    
    // find all the non-zero values (aka triplets) 
    std::vector<Eigen::Triplet<double> > trips;
    for (unsigned int i = 0; i < W.outerSize(); ++i) 
    {
        for ( Eigen::SparseMatrix<double>::InnerIterator it(W,i); it; ++it)
        {
            trips.push_back(Eigen::Triplet<double>(it.row(), it.col(), it.value()));
        }
    }    
    
    // save the number of triplets 
    //ofs << (long)trips.size();
    long ntrips = trips.size();
    ofs.write(reinterpret_cast<const char*>(&ntrips), sizeof(ntrips));
    
    // now save the triplets themselves
    for (unsigned int i = 0; i < trips.size(); i++)
    {
        Eigen::Triplet<double>& rt = trips[i];
        //ofs << (long)rt.row() << (long)rt.col() << (double)rt.value();
        long x = rt.row(), y = rt.col();
        double w = rt.value();

        ofs.write(reinterpret_cast<const char*>(&x), sizeof(x));
        ofs.write(reinterpret_cast<const char*>(&y), sizeof(y));
        ofs.write(reinterpret_cast<const char*>(&w), sizeof(w));

    }
    
    ofs.close();

    return true;
}

bool WeightCache::get(const std::string& key, Eigen::SparseMatrix<double>& W ) const
{
    AutoLock<Mutex> lock(mutex_);
    
    std::string fn = filename(key);

    eckit::LocalPathName fpath(fn);
    if(!fpath.exists())
        return false;
    
    std::ifstream ifs(fn.c_str(), std::ios::binary);
    if (!ifs.good())
        return false;

    Timer t("loading cached weights");

    // read inpts, outpts sizes of matrix
    long inner, outer;
    //ifs >> inner >> outer;
    ifs.read(reinterpret_cast<char*>(&inner), sizeof(inner));
    ifs.read(reinterpret_cast<char*>(&outer), sizeof(outer));

    long npts;
    ifs.read(reinterpret_cast<char*>(&npts), sizeof(npts));
    
    // read total sparse points of matrix (so we can reserve)
    std::vector<Eigen::Triplet<double> > insertions;
    
    insertions.reserve(npts);
    
    // read the values
    for (unsigned int i = 0; i < npts; i++)
    {
        long x, y;
        double w;
        //ifs >> x >> y >> w;
        ifs.read(reinterpret_cast<char*>(&x), sizeof(x));
        ifs.read(reinterpret_cast<char*>(&y), sizeof(y));
        ifs.read(reinterpret_cast<char*>(&w), sizeof(w));
        insertions.push_back(Eigen::Triplet<double>(x, y, w));
    }
    
    // modify the matrix only when we have enough values
    W = Eigen::SparseMatrix<double>(inner, outer);
    // set the weights from the triplets
    W.setFromTriplets(insertions.begin(), insertions.end());
    
    return true;
}

} // namespace mir
