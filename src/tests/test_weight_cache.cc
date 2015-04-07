/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#define BOOST_TEST_MODULE test_mir

#include "ecbuild/boost_test_framework.h"

#include <cstdio>

#include "eckit/log/Log.h"
#include "eckit/runtime/Tool.h"

#include "mir/WeightCache.h"

using namespace eckit;
using namespace mir;

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE( test_read_write )
{
    // We delete the cache file before we test
    char key[] = "test_matrix";
    ::remove(WeightCache::filename(key).asString().c_str());

    int n = 100, m = 200;
    // create a sparse matrix
    Weights::Matrix M(n, m);

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
    BOOST_CHECK(added);

    // chck that any additional adds fail
    BOOST_CHECK(!WeightCache::add(key, M));
    BOOST_CHECK(!WeightCache::add(key, M));


    // now get the data back again
    Weights::Matrix W(n, m);
    bool got = WeightCache::get(key, W);
    BOOST_CHECK(got);

    // now get the triplets from W and check them against M
    std::vector<Eigen::Triplet<double> > triplets;
    for (unsigned int i = 0; i < W.outerSize(); ++i)
    {
        for (Weights::Matrix::InnerIterator it(W,i); it; ++it)
        {
            triplets.push_back(Eigen::Triplet<double>(it.row(), it.col(), it.value()));
        }
    }

    // check construction of the matrix
    BOOST_CHECK(triplets.size() == insertions.size());
    BOOST_CHECK(W.size() == M.size());
    BOOST_CHECK(W.outerSize() == M.outerSize());
    BOOST_CHECK(W.innerSize() == M.innerSize());

    // check the values
    for (unsigned int i = 0; i < triplets.size(); i++)
    {
      BOOST_CHECK(triplets[i].col() == insertions[i].col());
      BOOST_CHECK(triplets[i].row() == insertions[i].row());
      BOOST_CHECK(triplets[i].value() == insertions[i].value());
    }

    // test that additional gets succeed
    BOOST_CHECK(WeightCache::get(key, W));
    BOOST_CHECK(WeightCache::get(key, W));

    // remove the file and BOOST_CHECK that get fails
    ::remove(WeightCache::filename(key).asString().c_str());
    BOOST_CHECK(!WeightCache::get(key, W));
    BOOST_CHECK(!WeightCache::get(key, W));
}

