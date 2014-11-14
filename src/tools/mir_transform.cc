#include <limits>
#include <cassert>
#include <sstream>
#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <memory>

#include "eckit/exception/Exceptions.h"
#include "eckit/filesystem/PathName.h"
#include "eckit/config/Resource.h"
#include "eckit/log/Timer.h"
#include "eckit/runtime/Tool.h"
#include "eckit/grib/GribHandle.h"
#include "eckit/grib/GribAccessor.h"

#include "atlas/atlas.h"
#include "atlas/FieldSet.h"
#include "atlas/meshgen/RGG.h"

#include "trans_api.h"

#include "mir/mir_config.h"
#include "mir/FieldSource.h"

//------------------------------------------------------------------------------------------------------

using namespace eckit;
using namespace eckit::grib;
using namespace atlas;
using namespace atlas::meshgen;
using namespace mir;

//------------------------------------------------------------------------------------------------------

class MirTransform : public eckit::Tool {

	void run()
	{
		Params& p = *ctxt_;

		GribHandle gh( params()["Input.Path"] );

		ASSERT( gh.gridType() == "sh" );

		long trunc = GribAccessor<long>("pentagonalResolutionParameterJ")(gh);

		DEBUG_VAR( trunc );
		DEBUG_VAR( gh.getDataValuesSize() );

		/// @todo temporary until Willem finishes the Grid factories in Atlas

		ASSERT( trunc > 0 );

		RGG* rgg = 0;

		// will be change to use factories
		switch( trunc ) {
			case 63:	rgg = new T63(); break;
			case 95:	rgg = new T95(); break;
			case 159:	rgg = new T159(); break;
			case 255:	rgg = new T255(); break;
			case 511:	rgg = new T511(); break;
			case 1279:	rgg = new T1279(); break;
			case 3999:	rgg = new T3999(); break;
			case 7999:	rgg = new T7999(); break;

			default:
				NOTIMP;
			break;
		}

		ASSERT( rgg );

		// prepare Trans object

		std::vector<int> nloen = rgg->nlon();

		Trans trans;

		trans.ndgl  = nloen.size();
		trans.nloen = nloen.data();

		long maxtr = params()["MaxTruncation"];

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

			if( gh.getDataValuesSize() == trans.nspec2g ) // full resolution
				gh.getDataValues(rspecg.data(),trans.nspec2g);
			else
			{
				std::vector<double> full( gh.getDataValuesSize() );
				gh.getDataValues(full.data(),full.size());
				NOTIMP;
			}
		}

		// Distribute data to all procs

		std::vector<double> rspec ( nfld * trans.nspec2  );

		DistSpec distspec = new_distspec(&trans);
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

		InvTrans invtrans = new_invtrans(&trans);
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

		GathGrid gathgrid = new_gathgrid(&trans);
		gathgrid.rgp  = rgp.data();
		gathgrid.rgpg = rgpg.data();
		gathgrid.nfld = nfld;
		gathgrid.nto  = nto.data();
		{
			Timer t("gather");
			trans_gathgrid(&gathgrid);
		}

		trans_finalize();


	}

public:

	MirTransform(int argc,char **argv): eckit::Tool(argc,argv)
	{
		trans_init();

//		atlas::atlas_init(argc,argv);

		MirContext* mir_ctxt = new MirContext();

		ValueParams* user( new ValueParams() );

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

		mir_ctxt->push_front( Params::Ptr(user) );
		ctxt_.reset( mir_ctxt );
	}

	Params& params() const { return *ctxt_; }

private:

	eckit::Params::Ptr ctxt_;

};

//------------------------------------------------------------------------------------------------------

int main( int argc, char **argv )
{
	MirTransform tool(argc,argv);
	tool.start();
	return 0;
}
