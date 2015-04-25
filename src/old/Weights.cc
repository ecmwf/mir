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

#include "atlas/Grid.h"
#include "atlas/Tesselation.h"

#include "mir/Weights.h"
#include "mir/WeightCache.h"

//------------------------------------------------------------------------------------------------------

using namespace atlas;

namespace mir {

//------------------------------------------------------------------------------------------------------

Weights::Weights()
{
}

Weights::~Weights()
{
}

void Weights::assemble(const Grid &in, const Grid &out, Weights::Matrix& W ) const
{
    WeightCache cache;
    const std::string whash = hash(in, out);
    bool wcached = cache.get( whash, W );
    if( ! wcached )
    {
        std::cout << ">>> computing weights ..." << std::endl;

        if( in.uid() == out.uid() && in.same(out) )
          W.setIdentity();
        else
          compute( const_cast<Grid&>(in), const_cast<Grid&>(out), W );

        cache.add( whash, W );
    }
    
}

std::string Weights::hash(const Grid &in, const Grid &out) const
{
    return classname() + std::string(".") +
           in.uid() + std::string(".") +
           out.uid();
}

//------------------------------------------------------------------------------------------------------

} // namespace mir
