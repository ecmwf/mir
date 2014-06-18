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

#include "eckit/maths/Eigen.h"

#include "atlas/grid/Grid.h"
#include "atlas/grid/Tesselation.h"
#include "atlas/mesh/FunctionSpace.hpp"

#include "mir/Masks.h"
#include "mir/WeightCache.h"
#include "mir/KNearest.h"

//------------------------------------------------------------------------------------------------------

using namespace Eigen;

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
    Grid& gi = const_cast<Grid&>(inp);

    Mesh& inp_mesh = gi.mesh();

    KNearest k4(4,gi);

    WeightMatrix wm;
    k4.assemble( mask.grid(), inp, wm ); // interpolate the input on the mask grid

    FieldHandle::Ptr mask_in( new FieldHandle( Grid::Ptr(&gi), inp_mesh.function_space("nodes").create_field<double>("mask_in",1) ) );
    {
        FieldT<double>& ifield = const_cast<FieldHandle&>(mask).data();
        FieldT<double>& ofield = mask_in->data();

        VectorXd::MapType fi = VectorXd::Map( ifield.data(), ifield.size() );
        VectorXd::MapType fo = VectorXd::Map( ofield.data(), ofield.size() );

        fo = wm * fi;
    }

//    k4.assemble( mask.grid(), inp, wm ); // interpolate the output on the mask grid

//    FieldHandle::Ptr mask_in( new FieldHandle( Grid::Ptr(&gi), inp_mesh.function_space("nodes").create_field<double>("mask_in",1) ) );
//    {
//        FieldT<double>& ifield = const_cast<FieldHandle&>(mask).data();
//        FieldT<double>& ofield = mask_in->data();

//        VectorXd::MapType fi = VectorXd::Map( ifield.data(), ifield.size() );
//        VectorXd::MapType fo = VectorXd::Map( ofield.data(), ofield.size() );

//        fo = wm * fi;
//    }

}

//------------------------------------------------------------------------------------------------------

} // namespace mir
