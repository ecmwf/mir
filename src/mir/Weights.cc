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
#include "atlas/grid/Grid.h"
#include "Weights.h"
#include "WeightCache.h"
#include "atlas/grid/Tesselation.h"


//-----------------------------------------------------------------------------
using atlas::grid::Grid;
using Eigen::SparseMatrix;
using atlas::grid::Tesselation;

namespace mir {

void WeightEngine::weights( Grid& in, Grid& out, SparseMatrix<double>& W ) const
{
    
    WeightCache cache;
    std::string whash = WeightEngine::weights_hash(in, out);
    bool wcached = cache.get( whash, W );
    if( ! wcached )
    {
        std::cout << ">>> computing weights ..." << std::endl;

        Tesselation::tesselate( in );

        compute( in.mesh(), out.mesh(), W );

        cache.add( whash, W );
    }
    
}


} // namespace mir
