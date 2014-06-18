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

#include "atlas/grid/FieldSet.h"

#include "mir/mir_config.h"

#include "mir/FieldSink.h"
#include "mir/FieldSource.h"
#include "mir/Interpolate.h"

//------------------------------------------------------------------------------------------------------

using namespace eckit;
using namespace atlas;
using namespace atlas::grid;
using namespace mir;

//------------------------------------------------------------------------------------------------------

class MirInterpolate : public eckit::Tool {

    virtual void run();

    Grid::Ptr make_grid( const std::string& filename );

public:

    MirInterpolate(int argc,char **argv): eckit::Tool(argc,argv)
    {
        PathName path_in;
        path_in = Resource<std::string>("-i","");
        if( path_in.asString().empty() )
            throw UserError( "missing input filename, parameter -i", Here());

        context_.set( "PathIn", Value(path_in) );

        PathName path_out;
        path_out = Resource<std::string>("-o","");
        if( path_out.asString().empty() )
            throw UserError( "missing output filename, parameter -o", Here());

        context_.set( "PathOut", Value(path_out) );

        PathName clone_path;
        clone_path = Resource<std::string>("-g","");
        if( clone_path.asString().empty() )
            throw UserError( "missing clone grid filename, parameter -g", Here());

        context_.set( "TargetGrid", Value(clone_path) );

        std::string method = Resource<std::string>("-m;$MIR_METHOD","fe");

        context_.set( "InterpolationMethod", method );
    }

private:

    Properties context_;

};

//------------------------------------------------------------------------------------------------------

void MirInterpolate::run()
{    
    std::cout.precision(std::numeric_limits< double >::digits10);
    std::cout << std::fixed;

    FieldSource source( context_ );
    Interpolate interpolator( context_ );
    FieldSink   sink( context_ );

    FieldSet::Ptr fs_inp = source.eval(); ASSERT( fs_inp );

    FieldSet::Ptr fs_out = interpolator.eval( fs_inp ); ASSERT( fs_out );

    sink.eval( fs_out );
}

//------------------------------------------------------------------------------------------------------

int main( int argc, char **argv )
{
    MirInterpolate tool(argc,argv);
    tool.start();
    return 0;
}

