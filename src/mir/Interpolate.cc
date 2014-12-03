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

#include "grib_api.h"

#include "eckit/exception/Exceptions.h"
#include "eckit/grib/GribHandle.h"
#include "eckit/log/Timer.h"
#include "eckit/log/Plural.h"
#include "eckit/utils/Translator.h"
#include "eckit/maths/Eigen.h"
#include "eckit/config/Resource.h"

#include "atlas/Grid.h"
#include "atlas/io/Grib.h"
#include "atlas/Tesselation.h"
#include "atlas/io/Gmsh.h"

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
using namespace eckit::grib;
using namespace atlas;
using namespace atlas::io;
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

	if( ::fclose(fh) == -1 )
		throw ReadError( std::string("error closing file ") + filename );

	GribHandle gh(h);
	Grid::Ptr g ( Grib::create_grid( gh ) );
    ASSERT( g );

    return g;
}

atlas::FieldSet::Ptr Interpolate::eval( const atlas::FieldSet::Ptr& fs_inp ) const
{
    ASSERT( fs_inp );

//    Grib::write( *fs_inp, "inp.grib" );

//    Params::Ptr rctxt( new FieldContext( fs_inp ) );

	/// @todo somewhere here we should use the GribParams* to pass to target_grid create...

    // clone grid

    Grid::Ptr target_grid;

    if( params().get("Target.GridPath").isNil() )
    {
		target_grid = Grid::create( eckit::UnScopeParams( "Target", params().self() ) );
	}
    else
    {
        target_grid = make_grid( params()["Target.GridPath"] );
    }

    ASSERT( target_grid );

    FieldSet::Ptr fs_out( new FieldSet( target_grid, fs_inp->field_names() ) );

    ASSERT( fs_out );

    size_t npts_inp = fs_inp->grid().npts();
    size_t npts_out = fs_out->grid().npts();

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

    // interpolation -- multiply interpolant matrix with field vector

    size_t nfields = fs_inp->size();

    if( nfields != fs_out->size() )
        throw SeriousBug( "Number of fields in input does not match number of fields in ouput", Here() );

    std::cout << ">>> interpolating " <<  Plural(nfields,"field")  << " ... " << std::endl;

    for( size_t n = 0; n < nfields; ++n )
    {
		Field& fi = (*fs_inp)[n];
		Field& fo = (*fs_out)[n];

        // interpolation
        {
            Timer t( "interpolating field " + Translator<size_t,std::string>()(n) );

			VectorXd::MapType vi = VectorXd::Map( fi.data<double>(), fi.size() );
			VectorXd::MapType vo = VectorXd::Map( fo.data<double>(), fo.size() );

			vo = W * vi;
        }

#ifdef ECKIT_HAVE_GRIB
        /// @todo this must be abstracted out, so GRIB is not exposed
        // metadata transfer by cloning the grib handle
        fo.grib( fi.grib().clone() );
#endif
    }

	// output to gmsh
	bool mirInterpolateDumpGmsh = Resource<bool>("mirInterpolateDumpGmsh;$MIR_INTERPOLATE_DUMP_GMSH");
	if( mirInterpolateDumpGmsh )
	{
		Grid& go = fs_out->grid();
		Tesselation::tesselate( go );

		std::cout << go.boundingBox() << std::endl;

		/* std::cout << go.mesh() << std::endl; */
		Gmsh::write3dsurf( go.mesh(), std::string("result.msh") );
	}

//    Grib::write( *fs_out, "out.grib" );

    return fs_out;
}

//------------------------------------------------------------------------------------------------------

} // namespace mir
