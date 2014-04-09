/*
 * (C) Copyright 1996-2012 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include <cstdio>

#include <Eigen/Sparse>

#include "eckit/log/Log.h"
#include "eckit/runtime/Tool.h"

#include "atlas/grid/Field.h"
#include "atlas/grid/Grid.h"
#include "atlas/grid/LatLon.h"

#include "mir/WeightCache.h"

using namespace eckit;
using namespace mir;

//-----------------------------------------------------------------------------

namespace eckit_test {

//-----------------------------------------------------------------------------

class TestWeightCache : public Tool {
public:

    TestWeightCache(int argc,char **argv): Tool(argc,argv) {}

    ~TestWeightCache() {}
    virtual void run();

    void test_constructor();
    void test_values();
};


void TestWeightCache::test_values()
{
    using namespace atlas::grid;
    
    // We delete the cache file before we test
    char key[] = "test_matrix";
    ::remove(WeightCache::filename(key).c_str());

    int n = 100, m = 200;
    // create a sparse matrix
    Eigen::SparseMatrix<double> M(n, m);

    // generate some sparse data
    std::vector<Eigen::Triplet<double> > insertions;

    int npts = 100;

    insertions.reserve(npts);

    for (unsigned int i = 0; i < npts; i++)
    {
        long x = i;
        long y = i+1;
        double w = (double)(x+y);;
        
        insertions.push_back(Eigen::Triplet<double>(x, y, w));
    }

    M.setFromTriplets(insertions.begin(), insertions.end());

    bool added = WeightCache::add(key, M);
    ASSERT(added);

    // ASSERT that any additional adds fail
    ASSERT(!WeightCache::add(key, M));
    ASSERT(!WeightCache::add(key, M));


    // now get the data back again
    Eigen::SparseMatrix<double> W(n, m);
    bool got = WeightCache::get(key, W);
    ASSERT(got);

    // now get the triplets from W and check them against M
    std::vector<Eigen::Triplet<double> > triplets;
    for (unsigned int i = 0; i < W.outerSize(); ++i) 
    {
        for (Eigen::SparseMatrix<double>::InnerIterator it(W,i); it; ++it)
        {
            triplets.push_back(Eigen::Triplet<double>(it.row(), it.col(), it.value()));
        }
    }    

    // check construction of the matrix
    ASSERT(triplets.size() == insertions.size());
    ASSERT(W.size() == M.size());
    ASSERT(W.outerSize() == M.outerSize());
    ASSERT(W.innerSize() == M.innerSize());
    
    // check the values
    for (unsigned int i = 0; i < triplets.size(); i++)
    {
        ASSERT(triplets[i].col() == insertions[i].col());
        ASSERT(triplets[i].row() == insertions[i].row());
        ASSERT(triplets[i].value() == insertions[i].value());
    }

    // test that additional gets succeed
    ASSERT(WeightCache::get(key, W));
    ASSERT(WeightCache::get(key, W));
    
    // remove the file and ASSERT that get fails
    ::remove(WeightCache::filename(key).c_str());
    ASSERT(!WeightCache::get(key, W));
    ASSERT(!WeightCache::get(key, W));



}

//-----------------------------------------------------------------------------

void TestWeightCache::run()
{
     test_values();
}

//-----------------------------------------------------------------------------

} // namespace eckit_test

//-----------------------------------------------------------------------------

int main(int argc,char **argv)
{
    eckit_test::TestWeightCache mytest(argc,argv);
    mytest.start();
    return 0;
}

