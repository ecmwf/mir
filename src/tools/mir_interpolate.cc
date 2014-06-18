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
<<<<<<< HEAD
#include "atlas/grid/GribRead.h"
#include "atlas/grid/GribWrite.h"
#include "atlas/grid/PointIndex3.h"
#include "atlas/grid/PointSet.h"
#include "atlas/grid/TriangleIntersection.h"
#include "atlas/grid/Tesselation.h"
#include "atlas/grid/Unstructured.h"

#include "mir/Bilinear.h"
#include "mir/FiniteElement.h"
#include "mir/KNearest.h"
#include "mir/WeightCache.h"
#include "mir/Weights.h"
=======
>>>>>>> 56da7bc9ed4290226573da769b307f875726839e

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

<<<<<<< HEAD
    // output grid + field

    std::cout << ">>> reading output grid ..." << std::endl;

    out_field = make_field( clone_grid, false );

    std::cout << "points " << out_field->grid().nPoints() << std::endl;

    // compute weights for each point in output grid

    Eigen::SparseMatrix<double> W( out_field->grid().nPoints(), in_field->grid().nPoints() );

    Weights* w;

    /// @todo make this into a factory
    if( method == std::string("fe") )
        w = new FiniteElement();
    if( method == std::string("kn") )
        w = new KNearest();
    if( method == std::string("bi") )
        w = new Bilinear();

    if( !w )
        throw UserError( std::string("Unknown Interpolator type ") + method , Here() );

    w->assemble( in_field->grid(), out_field->grid(), W );

    // interpolation -- multiply interpolant matrix with field vector

    std::cout << ">>> interpolating ..." << std::endl;

    FieldT<double>& ifield = in_field->data();
    FieldT<double>& ofield = out_field->data();

    {
        Timer t("interpolation");

        VectorXd::MapType fi = VectorXd::Map( ifield.data(), ifield.size() );
        VectorXd::MapType fo = VectorXd::Map( ofield.data(), ofield.size() );

        fo = W * fi;
    }

    // output to gmsh

    if( gmsh )
    {
        std::cout << ">>> output to gmsh" << std::endl;

        Tesselation::tesselate( in_field->grid() );

        atlas::Gmsh::write3dsurf( in_field->grid().mesh(), "input.msh" );

        Tesselation::tesselate( out_field->grid() );

        atlas::Gmsh::write3dsurf( out_field->grid().mesh(), "output.msh" );
    }

    // output to grib

    {
        Timer t("grib write");
        std::cout << ">>> output to grib" << std::endl;
        GribWrite::clone( *out_field, clone_grid, out_filename );
    }
=======
    FieldSet::Ptr fs_out = interpolator.eval( fs_inp ); ASSERT( fs_out );
>>>>>>> 56da7bc9ed4290226573da769b307f875726839e

    sink.eval( fs_out );
}

//------------------------------------------------------------------------------------------------------

int main( int argc, char **argv )
{
    MirInterpolate tool(argc,argv);
    tool.start();
    return 0;
}

