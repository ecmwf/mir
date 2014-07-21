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

#include "eckit/exception/Exceptions.h"
#include "eckit/grib/GribAccessor.h"
#include "eckit/grib/GribHandle.h"
#include "eckit/log/Timer.h"
#include "eckit/log/Plural.h"
#include "eckit/utils/Translator.h"
#include "eckit/maths/Eigen.h"

#include "atlas/grid/Grid.h"
#include "atlas/grid/Tesselation.h"
#include "atlas/grid/GribRead.h"
#include "atlas/grid/GribWrite.h"
#include "atlas/grid/GridFactory.h"

#include "mir/Bilinear.h"
#include "mir/FiniteElement.h"
#include "mir/Interpolate.h"
#include "mir/KNearest.h"
#include "mir/PseudoLaplace.h"
#include "mir/WeightCache.h"
#include "mir/Weights.h"
#include "mir/Masks.h"
#include "mir/FieldContext.h"

//------------------------------------------------------------------------------------------------------

using namespace Eigen;
using namespace eckit;
using namespace atlas;
using namespace atlas::grid;
using namespace mir;

namespace mir {

//------------------------------------------------------------------------------------------------------

Interpolate::Interpolate(const eckit::Params::Ptr& p) : Action(p)
{
}

Interpolate::~Interpolate()
{
}

static Grid::Ptr make_grid( const std::string& filename )
{
    FILE* fh = ::fopen( filename.c_str(), "r" );
    if( fh == 0 )
        throw ReadError( std::string("error opening file ") + filename );

    int err = 0;
    grib_handle* h;

    h = grib_handle_new_from_file(0,fh,&err);

    if( h == 0 || err != 0 )
        throw ReadError( std::string("error reading grib file ") + filename );

    Grid::Ptr g ( GribRead::create_grid_from_grib( h ) );
    ASSERT( g );

    grib_handle_delete(h);

    if( ::fclose(fh) == -1 )
        throw ReadError( std::string("error closing file ") + filename );

    return g;
}

Interpolate::FieldSet::Ptr Interpolate::eval( const Interpolate::FieldSet::Ptr& fs_inp ) const
{
    ASSERT( fs_inp );

    DEBUG_HERE;

    GribWrite::write( *fs_inp, "inp.grib" );

    DEBUG_HERE;

//    Params::Ptr rctxt( new FieldContext( fs_inp ) );

    // clone grid

    Grid::Ptr target_grid;

    if( params().get("Target.GridPath").isNil() )
    {
        target_grid = GridFactory::create( eckit::UnScopeParams( "Target", params().self() ) );
    }
    else
    {
        target_grid = make_grid( params()["Target.GridPath"] );
    }

    ASSERT( target_grid );

    FieldSet::Ptr fs_out( new FieldSet( target_grid, fs_inp->field_names() ) );

    ASSERT( fs_out );

    size_t npts_inp = fs_inp->grid().nPoints();
    size_t npts_out = fs_out->grid().nPoints();

    DEBUG_VAR( npts_inp );
    DEBUG_VAR( npts_out );

    DEBUG_VAR( params() );

    std::cout << ">>> interpolation points " << npts_inp << " -> " << npts_out << std::endl;

    // compute weights for each point in output grid

    Weights::Matrix W( npts_out, npts_inp );

    Weights* w;

    /// @todo make this into a factory
    std::string method = params()["InterpolationMethod"];
    if( method == std::string("fe") )
        w = new FiniteElement();
    if( method == std::string("kn") )
        w = new KNearest();
    if( method == std::string("plap") )
        w = new PseudoLaplace();
    if( method == std::string("bi") )
        w = new Bilinear();

    DEBUG_HERE;

    if( !w )
        throw UserError( std::string("Unknown Interpolator type ") + method , Here() );

    w->assemble( fs_inp->grid(), fs_out->grid(), W );

    // apply mask if necessary

    if( ! params().get("MaskPath").isNil() )
    {
        PathName mask_path = params()["MaskPath"];

        FieldSet::Ptr fmask( new FieldSet( mask_path ) ); ASSERT( fmask );

        if( fmask->size() != 1 )
            throw UserError( "User provided mask file with multiple fields", Here() );

        Masks m;
        m.assemble( (*fmask)[0], fs_inp->grid(), fs_out->grid(), W);
    }

    DEBUG_HERE;

    // interpolation -- multiply interpolant matrix with field vector

    size_t nfields = fs_inp->size();

    if( nfields != fs_out->size() )
        throw SeriousBug( "Number of fields in input does not match number of fields in ouput", Here() );

    std::cout << ">>> interpolating " <<  Plural(nfields,"field")  << " ... " << std::endl;

    for( size_t n = 0; n < nfields; ++n )
    {
        FieldHandle& fi = *(fs_inp->fields()[n]);
        FieldHandle& fo = *(fs_out->fields()[n]);

        FieldT<double>& ifield = fi.data();
        FieldT<double>& ofield = fo.data();

        // interpolation
        {
            Timer t( "interpolating field " + Translator<size_t,std::string>()(n) );

            VectorXd::MapType fi = VectorXd::Map( ifield.data(), ifield.size() );
            VectorXd::MapType fo = VectorXd::Map( ofield.data(), ofield.size() );

            fo = W * fi;
        }

        /// @todo this must be abstracted out, so GRIB is not exposed
        // metadata transfer by cloning the grib handle
        fo.grib( fi.grib().clone() );
    }


    DEBUG_HERE;

    GribWrite::write( *fs_out, "out.grib" );

    DEBUG_VAR( fs_out->size() );

    return fs_out;
}

//------------------------------------------------------------------------------------------------------

} // namespace mir
