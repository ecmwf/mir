#include <limits>
#include <cassert>
#include <sstream>
#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <memory>

#include "atlas/io/Gmsh.hpp"
#include "atlas/mesh/Mesh.hpp"

#include "eckit/exception/Exceptions.h"
#include "eckit/config/Resource.h"
#include "eckit/runtime/Tool.h"
#include "eckit/grib/GribAccessor.h"

#include "atlas/grid/FieldSet.h"
#include "atlas/grid/GribRead.h"
#include "atlas/grid/GribWrite.h"
#include "atlas/grid/PointIndex3.h"
#include "atlas/grid/PointSet.h"
#include "atlas/grid/TriangleIntersection.h"
#include "atlas/grid/Tesselation.h"
#include "atlas/grid/Unstructured.h"

#include "mir/FiniteElement.h"
#include "mir/KNearest.h"
#include "mir/WeightCache.h"
#include "mir/Weights.h"

//------------------------------------------------------------------------------------------------------

#if 1
#define DBG     std::cout << Here() << std::endl;
#define DBGX(x) std::cout << #x << " -> " << x << std::endl;
#else
#define DBG
#define DBGX(x)
#endif

//------------------------------------------------------------------------------------------------------

using namespace Eigen;
using namespace eckit;
using namespace atlas;
using namespace atlas::grid;
using namespace mir;

//------------------------------------------------------------------------------------------------------

class MirInterpolate : public eckit::Tool {

    virtual void run();

    FieldHandle::Ptr make_field( const std::string& filename, bool read_field = true );

public:

    MirInterpolate(int argc,char **argv): eckit::Tool(argc,argv)
    {
        gmsh = Resource<bool>("-gmsh",false);

        in_filename = Resource<std::string>("-i","");
        if( in_filename.empty() )
            throw UserError( "missing input filename, parameter -i", Here());

        out_filename = Resource<std::string>("-o","");
        if( out_filename.empty() )
            throw UserError( "missing output filename, parameter -o", Here());

        clone_grid = Resource<std::string>("-g","");
        if( clone_grid.empty() )
            throw UserError( "missing clone grid filename, parameter -g", Here());

        method = Resource<std::string>("-m;$MIR_METHOD","fe");
    }

private:

    bool gmsh;
    std::string in_filename;
    std::string out_filename;
    std::string clone_grid;
    std::string method;
};

//------------------------------------------------------------------------------------------------------

static GribAccessor<std::string> grib_shortName("shortName");

FieldHandle::Ptr MirInterpolate::make_field( const std::string& filename, bool read_field )
{
    FILE* fh = ::fopen( filename.c_str(), "r" );
    if( fh == 0 )
        throw ReadError( std::string("error opening file ") + filename );

    int err = 0;
    grib_handle* h;

    h = grib_handle_new_from_file(0,fh,&err);

    if( h == 0 || err != 0 )
        throw ReadError( std::string("error reading grib file ") + filename );

    Grid::Ptr g ( GribRead::create_grid_from_grib(h) );

    const std::string sname = grib_shortName(h);

    Mesh& mesh = g->mesh();

    FunctionSpace&  nodes  = mesh.function_space( "nodes" );

    if( read_field )
        GribRead::read_field_from_grib(h,mesh,sname);
    else
        nodes.create_field<double>(sname,1);

    grib_handle_delete(h);
    if( ::fclose(fh) == -1 )
        throw ReadError( std::string("error closing file ") + filename );

    // finalize FieldHandle

    FieldHandle::Ptr hf( new FieldHandle( g, nodes.field<double>( sname ) ) );

    return hf;
}

//------------------------------------------------------------------------------------------------------

void MirInterpolate::run()
{    
    std::cout.precision(std::numeric_limits< double >::digits10);
    std::cout << std::fixed;

    FieldHandle::Ptr in_field;
    FieldHandle::Ptr out_field;

    // input grid + field

    std::cout << ">>> reading input grid + field ..." << std::endl;

    in_field = make_field( in_filename );

    std::cout << "points " << in_field->grid().nPoints() << std::endl;

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

//    std::cout << ">>> deleting input field" << std::endl;
//    in_field.reset();
//    std::cout << ">>> deleting output field" << std::endl;
//    out_field.reset();
}

//------------------------------------------------------------------------------------------------------

int main( int argc, char **argv )
{
    MirInterpolate tool(argc,argv);
    tool.start();
    return 0;
}

