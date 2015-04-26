#include <limits>
#include <cassert>
#include <sstream>
#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <memory>

#include "eckit/config/Resource.h"
#include "eckit/exception/Exceptions.h"
#include "eckit/filesystem/PathName.h"
#include "eckit/grib/GribAccessor.h"
#include "eckit/grib/GribMutator.h"
#include "eckit/grib/GribHandle.h"
#include "eckit/io/DataHandle.h"
#include "eckit/log/Timer.h"
#include "eckit/runtime/Tool.h"

#include "atlas/atlas_config.h"
#include "atlas/FieldSet.h"
#include "atlas/grids/grids.h"
#include "atlas/io/Grib.h"

#include "transi/trans.h"

#include "mir/mir_config.h"
#include "mir/FieldSource.h"

//------------------------------------------------------------------------------------------------------

using namespace eckit;
using namespace eckit::grib;
using namespace atlas;
using namespace atlas::io;
using namespace mir;

//------------------------------------------------------------------------------------------------------

class MirTransform : public eckit::Tool {

	void run()
	{
		Params p(ctxt_);

		GribHandle in_grib( p["Input.Path"] );

		DEBUG_VAR( in_grib.gridType() );

//		std::vector<long> pl = GribAccessor< std::vector<long> >("pl")(gh);
//		DEBUG_VAR( pl );


		ASSERT( in_grib.gridType() == "sh" );

		long trunc = GribAccessor<long>("pentagonalResolutionParameterJ")(in_grib);

		DEBUG_VAR( trunc );
		DEBUG_VAR( in_grib.getDataValuesSize() );

		/// @todo temporary until Willem finishes the Grid factories in Atlas

		ASSERT( trunc > 0 );

		eckit::ScopedPtr<grids::ReducedGaussianGrid> rgg;

		// will be change to use factories
		switch( trunc ) {
			case 63:	rgg.reset(  new grids::rgg::N32() ); break;
			case 95:	rgg.reset(  new grids::rgg::N48() ); break;
			case 159:	rgg.reset(  new grids::rgg::N80() ); break;
			case 255:	rgg.reset(  new grids::rgg::N128() ); break;
			case 511:	rgg.reset(  new grids::rgg::N256() ); break;
			case 1279:	rgg.reset(  new grids::rgg::N640() ); break;
			case 3999:	rgg.reset(  new grids::rgg::N2000() ); break;
			case 7999:	rgg.reset(  new grids::rgg::N4000() ); break;

			default:
				NOTIMP;
			break;
		}

		long gaussN = (trunc + 1) / 2;  // assumption: linear grid

		DEBUG_VAR( gaussN );

		ASSERT( rgg );

		// prepare Trans object

		struct Trans_t trans = new_trans();

	    trans.ndgl  = rgg->npts_per_lat().size();
    	trans.nloen = (int*) malloc( trans.ndgl * sizeof(int) ); ///< allocate array to be freed in trans_delete()

    	::memcpy( trans.nloen, &(rgg->npts_per_lat()[0]), sizeof(int)*trans.ndgl );
    	
		long maxtr = p["MaxTruncation"];

		trans.nsmax = maxtr ? maxtr : (2*trans.ndgl-1)/2; // assumption: linear grid

		DEBUG_VAR( trans.ndgl );
//		DEBUG_VAR( nloen );
		DEBUG_VAR( trans.nsmax );

		// register resolution in trans library
		{
			Timer t("setup");
			trans_setup( &trans );
		}

		DEBUG_VAR( trans.nspec2g );

		int nfld = 1; // number of fields

		DEBUG_VAR( nfld );

		std::vector<int> nfrom(nfld,1); // processors responsible for distributing each field

		std::vector<double> rspecg;

		if( trans.myproc == 1 )
		{
			Timer t("unpack");

			rspecg.resize( nfld * trans.nspec2g ); // Global spectral array

			if( in_grib.getDataValuesSize() == trans.nspec2g ) // full resolution
				in_grib.getDataValues(rspecg.data(),trans.nspec2g);
			else
			{
				std::vector<double> full( in_grib.getDataValuesSize() );
				in_grib.getDataValues(full.data(),full.size());
				NOTIMP;
			}
		}

		// Distribute data to all procs

		std::vector<double> rspec ( nfld * trans.nspec2  );

		struct DistSpec_t distspec = new_distspec(&trans);
		distspec.nfrom  = nfrom.data();
		distspec.rspecg = rspecg.data();
		distspec.rspec  = rspec.data();
		distspec.nfld   = nfld;
		{
			Timer t("distribute");
			trans_distspec(&distspec);
		}

		// Transform sp to gp fields

		std::vector<double> rgp ( nfld * trans.ngptot );

		struct InvTrans_t invtrans = new_invtrans(&trans);
		invtrans.nscalar   = nfld;
		invtrans.rspscalar = rspec.data();
		invtrans.rgp       = rgp.data();
		{
			Timer t("transform");
			trans_invtrans(&invtrans);
		}

		// Gather all gridpoint fields

		std::vector<double> rgpg;
		if( trans.myproc == 1 )
		  rgpg.resize( nfld * trans.ngptotg );

		std::vector<int> nto ( nfld );
		for( int jfld=0; jfld<nfld; ++jfld )
		  nto[jfld] = 1;

		struct GathGrid_t gathgrid = new_gathgrid(&trans);
		gathgrid.rgp  = rgp.data();
		gathgrid.rgpg = rgpg.data();
		gathgrid.nfld = nfld;
		gathgrid.nto  = nto.data();
		{
			Timer t("gather");
			trans_gathgrid(&gathgrid);
		}

		// create the Grid & Field

		std::string grid_uid = "reduced_gg.N" + eckit::Translator<long,std::string>()(gaussN);

		Grid::Ptr grid( Grid::create(grid_uid) );

		FunctionSpace& nodes_out = grid->mesh().function_space( "nodes" );
		Field& f = nodes_out.create_field<double>("theone",1);

		std::cout << grid->bounding_box() << std::endl;

		DEBUG_VAR( f.size() );
		DEBUG_VAR( trans.ngptotg );

		ASSERT( f.size() == trans.ngptotg );

//		for( size_t i = 0; i < trans.ngptotg; ++i )
//			std::cout << i << " " << gathgrid.rgpg[i] << std::endl;

		::memcpy( f.data<double>(), gathgrid.rgpg, sizeof(double)*trans.ngptotg );

//		for( size_t i = 0; i < f.size(); ++i )
//			std::cout << i << " " << f.data<double>()[i] << std::endl;

		/// out put to Grib

		GribHandle::Ptr good_grid = io::Grib::create_handle( *grid, in_grib.edition() ); // correct grid
		f.grib( io::Grib::copy_metadata( in_grib, *good_grid ) ); // copy metadata
		ASSERT( f.size() == f.grib()->getDataValuesSize() );
		f.grib()->setDataValues( f.data<double>(), f.size() );

		// f.grib( in_grib.clone() ); // gets the correct metadata
		// GribHandle::Ptr ogh ( Grib::clone(f, *io::Grib::create_handle( *grid, in_grib.edition() ) ) ); // clones into the correct grid

		// dump the GRIB to the DataHandle

		PathName fpath( p["Target.Path"].as<std::string>() );
		DataHandle* dh = fpath.fileHandle();

		dh->openForWrite(0);

		f.grib()->write(*dh);

		dh->close();

		trans_delete(&trans);
		trans_finalize();
	}

public:

	MirTransform(int argc,char **argv): eckit::Tool(argc,argv)
	{
		trans_init();

//		atlas::atlas_init(argc,argv);

		Properties* user( new Properties() );

		PathName path_in;
		path_in = Resource<std::string>("-i","");
		if( path_in.asString().empty() )
			throw UserError( "missing input filename, parameter -i", Here());

		user->set( "Input.Path", Value(path_in) );

		PathName path_out;
		path_out = Resource<std::string>("-o","");
		if( path_out.asString().empty() )
			throw UserError( "missing output filename, parameter -o", Here());

		user->set( "Target.Path", Value(path_out) );

		long maxtr = Resource<long>("-t",0);
		if( maxtr < 0 )
			throw UserError( "Max truncation cannot be negative", Here() );

		user->set( "MaxTruncation", maxtr );

        trans_finalize();

	}

private:

  MirParams ctxt_;

};

//------------------------------------------------------------------------------------------------------

int main( int argc, char **argv )
{
	MirTransform tool(argc,argv);
	tool.start();
	return 0;
}
