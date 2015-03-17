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

#include "eckit/config/Resource.h"
#include "eckit/exception/Exceptions.h"
#include "eckit/grib/GribHandle.h"
#include "eckit/log/Timer.h"
#include "eckit/log/Plural.h"
#include "eckit/maths/Eigen.h"
#include "eckit/utils/Translator.h"
#include "eckit/value/UnScopeParams.h"

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
#include "mir/FieldParams.h"

//------------------------------------------------------------------------------------------------------

using namespace Eigen;
using namespace eckit;
using namespace atlas;
using namespace eckit::grib;
using namespace mir;

namespace mir {

//------------------------------------------------------------------------------------------------------

Interpolate::Interpolate(const Params & p) : Action(p)
{
}

Interpolate::~Interpolate()
{
}

void Interpolate::applyMask(const atlas::Grid& grid_inp, const atlas::Grid& grid_out, Weights::Matrix& W ) const
{
  if( params().has("MaskPath") )
  {
      PathName mask_path = params()["MaskPath"];

      FieldSet::Ptr fmask( new FieldSet( mask_path ) ); ASSERT( fmask );

      if( fmask->size() != 1 )
          throw UserError( "User provided mask file with multiple fields", Here() );

      Masks m;
      m.assemble( (*fmask)[0], grid_inp, grid_out, W );
  }
}

atlas::FieldSet::Ptr Interpolate::eval( const atlas::FieldSet::Ptr& fs_inp ) const
{
    ASSERT( fs_inp );

//    Grib::write( *fs_inp, "inp.grib" );

//    Params::Ptr rctxt( new FieldParams( fs_inp ) );

    /// @todo somewhere here we should use the GribParams* to pass to create grid_out ...

    // clone grid

    Grid::Ptr target_grid(
          !params().has("Target.GridPath") ?
            Grid::create( Params( UnScopeParams( "Target", params() ) ) ) :
            atlas::io::make_grid( params()["Target.GridPath"] ).get() );

    ASSERT( target_grid );

    FieldSet::Ptr fs_out( new FieldSet() );

    ASSERT( fs_out );

    FunctionSpace& nodes_out = target_grid->mesh().function_space( "nodes" );

    for (size_t n=0; n<fs_inp->size(); ++n)
    {
      const Field& f_in = (*fs_inp)[n];
      Field& f = nodes_out.create_field<double>(f_in.name(),1);
      fs_out->add_field(f.self());
    }

    ASSERT( fs_inp->size() == fs_out->size() );

    /// @todo make this into a factory
    std::string method = params()["InterpolationMethod"];
    Weights* w = method=="fe"?   new FiniteElement() :
                 method=="kn"?   new KNearest() :
                 method=="plap"? new PseudoLaplace() :
                 method=="bi"?   new Bilinear() :
                                 (Weights*) NULL;
    if( !w )
        throw UserError( std::string("Unknown Interpolator type ") + method , Here() );

    for (size_t n = 0; n < fs_inp->size(); ++n)
    {
      Field& fi = (*fs_inp)[n];
      Field& fo = (*fs_out)[n];

      const Grid& grid_inp = fi.grid();
      const Grid& grid_out = *target_grid;

      size_t npts_inp = grid_inp.npts();
      size_t npts_out = grid_out.npts();

      std::cout << ">>> interpolation points " << npts_inp << " -> " << npts_out << std::endl;

      // compute weights for each point in output grid

      Weights::Matrix W( npts_out, npts_inp );

      w->assemble( grid_inp, grid_out, W );

      // apply mask if necessary

      applyMask(grid_inp, grid_out, W);

      // interpolation -- multiply interpolant matrix with field vector

      {
        Timer t( "interpolating field " + Translator<size_t,std::string>()(n) );

        VectorXd::MapType vi = VectorXd::Map( fi.data<double>(), fi.size() );
        VectorXd::MapType vo = VectorXd::Map( fo.data<double>(), fo.size() );

        vo = W * vi;
      }

#ifdef ECKIT_HAVE_GRIB
      /// @todo this must be abstracted out, so GRIB is not exposed
      ///       here the metadata is transfered by cloning the grib handle
      if( fi.grib() )
        fo.grib( fi.grib()->clone() );
#endif

    }

    // output to gmsh
    bool mirInterpolateDumpGmsh = Resource<bool>("mirInterpolateDumpGmsh;$MIR_INTERPOLATE_DUMP_GMSH",false);
    if( mirInterpolateDumpGmsh )
    {
        Tesselation::tesselate( *target_grid );

        /* std::cout << go.mesh() << std::endl; */
        atlas::io::Gmsh::write3dsurf( target_grid->mesh(), std::string("result.msh") );
    }

//    Grib::write( *fs_out, "out.grib" );

	return fs_out;
}

ExpPtr interpolate(const ExpPtr& e)
{
	NOTIMP;
//	return ExpPtr( new Interpolate( e ) );
}

//------------------------------------------------------------------------------------------------------

} // namespace mir
