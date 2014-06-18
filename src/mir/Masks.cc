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
#include "atlas/grid/Tesselation.h"

#include "mir/Masks.h"
#include "mir/WeightCache.h"
#include "mir/KNearest.h"

//------------------------------------------------------------------------------------------------------

using namespace atlas;
using namespace atlas::grid;

namespace mir {

//------------------------------------------------------------------------------------------------------

Masks::Masks()
{
}

Masks::~Masks()
{
}

void Masks::assemble(const FieldHandle& mask, const Grid& inp, const Grid& out, Masks::WeightMatrix& W) const
{
//    Mesh& mask_mesh = mask.grid().mesh();

    KNearest k4(4);


}

//------------------------------------------------------------------------------------------------------

} // namespace mir
