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

void Masks::assemble(const FieldHandle& mask, const Grid& inp, const Grid& out, Weights::Matrix& W) const
{
    Grid& gm = const_cast<Grid&>(mask.grid());

    KNearest k4(4,gm);

    // interpolate the input on the mask grid
    Grid& gi = const_cast<Grid&>(inp);
    Mesh& inp_mesh = gi.mesh();
    FieldHandle::Ptr mask_inp( new FieldHandle( Grid::Ptr(&gi), inp_mesh.function_space("nodes").create_field<double>("mask_inp",1) ) );
    {
        FieldT<double>& ifield = const_cast<FieldHandle&>(mask).data();
        FieldT<double>& ofield = mask_inp->data();

        Weights::Matrix wm( ofield.size(), ifield.size() );
        k4.assemble( mask.grid(), inp, wm );

        VectorXd::MapType fi = VectorXd::Map( ifield.data(), ifield.size() );
        VectorXd::MapType fo = VectorXd::Map( ofield.data(), ofield.size() );

        fo = wm * fi;
    }

    // interpolate the output on the mask grid
    Grid& go = const_cast<Grid&>(out);
    Mesh& out_mesh = go.mesh();
    FieldHandle::Ptr mask_out( new FieldHandle( Grid::Ptr(&go), out_mesh.function_space("nodes").create_field<double>("mask_out",1) ) );
    {
        FieldT<double>& ifield = const_cast<FieldHandle&>(mask).data();
        FieldT<double>& ofield = mask_out->data();

        Weights::Matrix wm( ofield.size(), ifield.size() );
        k4.assemble( mask.grid(), out, wm );

        VectorXd::MapType fi = VectorXd::Map( ifield.data(), ifield.size() );
        VectorXd::MapType fo = VectorXd::Map( ofield.data(), ofield.size() );

        fo = wm * fi;
    }

    VectorXd ws;    //< inneficient
    VectorXd imask; //< inneficient

    for( size_t k = 0; k < W.outerSize(); ++k )
    {
//        std::cout << "-----------------------" << std::endl;

        double omask = (*mask_out).data()[k];
        bool mask1 = ( omask >= 0.5 );

        /// @todo this probably can be done better -- how can we know how many cols() in a row() without iterating
        size_t ncols = 0;
        for( Weights::Matrix::InnerIterator it(W,k); it; ++it )
        {
//            assert( k == it.row() && it.index() == it.col() );
            ++ncols;
        //          std::cout << it.value() << " "
        //                    << it.row()   << " "        // row index -- should be equal to k
        //                    << it.col()   << " "        // col index
        //                    << it.index() << std::endl; // inner index, here it is equal to it.col()
        }

        ws.resize(ncols);
        imask.resize(ncols);

        size_t j = 0;
        for( Weights::Matrix::InnerIterator it(W,k); it; ++it, ++j )
        {
            ws[j] =  it.value();
            imask[j] = (*mask_inp).data()[it.col()];
        }

//        std::cout << ws.transpose() << std::endl;

        bool modified = false;
        for( size_t jc = 0; jc < ncols; ++jc )
        {
            if( ( mask1 && imask[jc] < 0.5 ) || ( !mask1 && imask[jc] >= 0.5 ) )
            {
                modified = true;
                ws[jc] *= 0.2;
            }
        }

        if( !modified ) continue;

//        std::cout << ws.transpose() << std::endl;

        const double wsum = ws.sum();

        ws.array() /= wsum;

        j = 0;
        for( Weights::Matrix::InnerIterator it(W,k); it; ++it, ++j )
        {
            it.valueRef() = ws[j];
        }

//        std::cout << ws.transpose() << std::endl;
    }

//    ::exit(0);

}

//------------------------------------------------------------------------------------------------------

} // namespace mir
