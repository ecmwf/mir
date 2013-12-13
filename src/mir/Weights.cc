/*
 * (C) Copyright 1996-2013 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "eckit/grid/Grid.h"
#include "eckit/log/Log.h"
#include <string>
#include "Weights.h"


using eckit::grid::Point2D;

//-----------------------------------------------------------------------------

namespace mir {


//-----------------------------------------------------------------------------
InverseSquare::InverseSquare() 
{
    eckit::Log::info() << "Build a InverseSquare" << std::endl;
}

InverseSquare::~InverseSquare() 
{
    eckit::Log::info() << "Destroy a InverseSquare" << std::endl;
}

void InverseSquare::generate(const Point2D& ref, const std::vector<Point2D>& closests, std::vector<double>& weights) const
{
    weights.resize(closests.size(), 0.0);
    double sum = 0.0;

    for (unsigned int j = 0; j < closests.size(); j++)
    {
        double delta = Point2D::distance(ref, closests[j]);
        /// @todo take epsilon from some general config
        weights[j] = 1.0 / (Point2D::EPSILON + (delta * delta));
        sum += weights[j];
    }

    // now normalise these
    for (unsigned int j = 0; j < closests.size(); j++)
    {
        ASSERT(sum != 0.0);
        weights[j] /= sum;
    }

}
} // namespace mir
