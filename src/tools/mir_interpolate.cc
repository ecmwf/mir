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
#include "eckit/runtime/Tool.h"
#include "eckit/xpr/Xpr.h"
#include "eckit/xpr/Map.h"

#include "mir/mir_config.h"
#include "mir/FieldSink.h"
#include "mir/FieldSource.h"
#include "mir/FieldSet.h"
#include "mir/Interpolate.h"

//------------------------------------------------------------------------------------------------------

using namespace eckit;
using namespace atlas;
using namespace mir;

//------------------------------------------------------------------------------------------------------

class MirInterpolate : public eckit::Tool {

	void run()
	{
		FieldSource source( ctxt_ );
		Interpolate interpolator( ctxt_ );
		FieldSink   sink( ctxt_ );

		FieldSet::Ptr fs_inp = source.eval(); ASSERT( fs_inp );

		FieldSet::Ptr fs_out = interpolator.eval( fs_inp ); ASSERT( fs_out );

		sink.eval( fs_out );
#if 0
		using eckit;

		MIRFactory


		FieldSource source( ctxt_ );
		Interpolate interpolator( ctxt_ );
		FieldSink   sink( ctxt_ );


		Xpr f1 = sink( interpolator( source() ));

		f1.eval();

		FieldSet::Ptr fs_inp = source.eval(); ASSERT( fs_inp );

		FieldSet::Ptr fs_out = interpolator.eval( fs_inp ); ASSERT( fs_out );

		sink.eval( fs_out );

		////----------------------------------------------------

		Xpr f1 = sink( interpolator( source() )) << c;
			f1 << c2;

		f1.bind(c);

		f1.eval();

		////----------------------------------------------------


		xpr::Xpr f = field_sink( ctxt_,
						xpr::map( interpolate(ctxt_), field_source( ctxt_ ) )
					);

		auto fs = field_sink(c);

		xpr::Xpr f2 = fs( xpr::map(c)( interpolate(c), field_source(c) ));

		std::cout << f << std::endl;

		std::cout << f << std::endl;

		f.eval();
#endif
	}

public:

	MirInterpolate(int argc,char **argv) :
		eckit::Tool(argc,argv),
		ctxt_( new MirContext() )
    {
        ValueParams* user( new ValueParams() );

        PathName path_in;
        path_in = Resource<std::string>("-i","");
        if( path_in.asString().empty() )
            throw UserError( "missing input filename, parameter -i", Here());

        user->set( "Input.Path", Value(path_in) );

        std::string oformat = Resource<std::string>("-oformat","grib");
        if( oformat.empty() )
            throw UserError( "bad output format, parameter -oformat", Here());

        user->set( "Target.OutputFormat", Value(oformat) );

        PathName path_out;
        path_out = Resource<std::string>("-o","");
        if( path_out.asString().empty() )
            throw UserError( "missing output filename, parameter -o", Here());

        user->set( "Target.Path", Value(path_out) );

        PathName clone_path;
        clone_path = Resource<std::string>("-g","");
        if( clone_path.asString().empty() )
            throw UserError( "missing clone grid filename, parameter -g", Here());

        user->set( "Target.GridPath", Value(clone_path) );

        std::string method = Resource<std::string>("-m;$MIR_METHOD","fe");

        user->set( "InterpolationMethod", method );

        static_cast<MirContext*>(ctxt_.get())->push_front( Params::Ptr(user) );
    }

private:

    eckit::Params::Ptr ctxt_;

};

//------------------------------------------------------------------------------------------------------

int main( int argc, char **argv )
{
    MirInterpolate tool(argc,argv);
    tool.start();
    return 0;
}

