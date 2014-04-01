#include <limits>
#include <cassert>
#include <sstream>
#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <memory>

#include "atlas/Gmsh.hpp"
#include "atlas/Mesh.hpp"

#include "eckit/exception/Exceptions.h"
#include "eckit/config/Resource.h"
#include "eckit/runtime/Tool.h"
#include "eckit/grib/GribAccessor.h"

#include "atlas/grid/GribRead.h"
#include "atlas/grid/GribWrite.h"
#include "atlas/grid/PointIndex3.h"
#include "eckit/geometry/PointSet.h"
#include "atlas/grid/TriangleIntersection.h"
#include "atlas/grid/Tesselation.h"

#include "mir/FEInterpolator.h"
#include "mir/WeightCache.h"

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
using namespace mir;

//------------------------------------------------------------------------------------------------------

static GribAccessor<long> edition("edition");
static GribAccessor<std::string> md5Section2("md5Section2");
static GribAccessor<std::string> md5Section3("md5Section3");

//------------------------------------------------------------------------------------------------------

std::string grib_hash( grib_handle* h )
{
    ASSERT(h);

    /// @todo create a 'geographyMd5'  accessor

    std::string md5;

    switch( edition(h) )
    {
    case 1:
        md5 = md5Section2(h);
        break;
    case 2:
        md5 = md5Section3(h);
        break;

    default:
        ASSERT( !md5.empty() );
        break;
    }

    return md5;
}

std::string grib_hash( const std::string& fname )
{
    FILE* fh = ::fopen( fname.c_str(), "r" );
    if( fh == 0 )
        throw ReadError( std::string("error opening file ") + fname );

    int err = 0;
    grib_handle* h = grib_handle_new_from_file(0,fh,&err);

    if( h == 0 || err != 0 )
        throw ReadError( std::string("error reading grib file ") + fname );

    std::string md5 = grib_hash(h);

    grib_handle_delete(h);

    if( ::fclose(fh) == -1 )
        throw ReadError( std::string("error closing file ") + fname );

    return md5;
}

std::string weights_hash( const std::string& in, const std::string& out )
{
    std::string in_md5  = grib_hash(in);
    std::string out_md5 = grib_hash(out);
    return in_md5 + std::string(".") + out_md5;
}

//------------------------------------------------------------------------------------------------------

class MirInterpolate : public eckit::Tool {

    virtual void run();

    void grib_load( const std::string& fname, atlas::Mesh& mesh, bool read_field = true );

public:

    MirInterpolate(int argc,char **argv): eckit::Tool(argc,argv)
    {
        gmsh = Resource<bool>("-gmsh",false);

        in_filename = Resource<std::string>("-i","");
        if( in_filename.empty() )
            throw UserError(Here(),"missing input filename, parameter -i");

        out_filename = Resource<std::string>("-o","");
        if( out_filename.empty() )
            throw UserError(Here(),"missing output filename, parameter -o");

        clone_grid = Resource<std::string>("-g","");
        if( clone_grid.empty() )
            throw UserError(Here(),"missing clone grid filename, parameter -g");
    }

private:

    bool gmsh;
    std::string in_filename;
    std::string out_filename;
    std::string clone_grid;
};

//------------------------------------------------------------------------------------------------------

/// @todo this will become an expression object
void MirInterpolate::grib_load( const std::string& fname, atlas::Mesh& mesh, bool read_field )
{
    FILE* fh = ::fopen( fname.c_str(), "r" );
    if( fh == 0 )
        throw ReadError( std::string("error opening file ") + fname );

    int err = 0;
    grib_handle* h;

    {
        Timer t("grib_handle_new_from_file");
        h = grib_handle_new_from_file(0,fh,&err);
    }

    if( h == 0 || err != 0 )
        throw ReadError( std::string("error reading grib file ") + fname );

    {
        Timer t("read_nodes_from_grib");
        GribRead::read_nodes_from_grib( h, mesh );
    }

    if( read_field )
    {
        Timer t("read_field_from_grib");
        GribRead::read_field_from_grib( h, mesh, "field" );
    }

    grib_handle_delete(h);

    // close file handle

    if( ::fclose(fh) == -1 )
        throw ReadError( std::string("error closing file ") + fname );
}

//------------------------------------------------------------------------------------------------------

void MirInterpolate::run()
{    
    std::cout.precision(std::numeric_limits< double >::digits10);
    std::cout << std::fixed;

    // input grid + field

    std::cout << ">>> reading input grid + field ..." << std::endl;

    std::unique_ptr< atlas::Mesh > in_mesh ( new Mesh() );

    {
        Timer t("grib read");
        grib_load( in_filename, *in_mesh );
    }

    FunctionSpace&  i_nodes = in_mesh->function_space( "nodes" );
    FieldT<double>& ifield = i_nodes.field<double>("field");

    const size_t nb_i_nodes = i_nodes.bounds()[1];

    std::cout << "points " << nb_i_nodes << std::endl;

    // output grid + field

    std::cout << ">>> reading output grid ..." << std::endl;

    std::unique_ptr< atlas::Mesh > out_mesh ( new Mesh() );

    grib_load( clone_grid, *out_mesh, false );

    FunctionSpace&  o_nodes = out_mesh->function_space( "nodes" );
    FieldT<double>& ofield = o_nodes.create_field<double>("field",1);

    const size_t nb_o_nodes = o_nodes.bounds()[1];

    std::cout << "points " << nb_o_nodes << std::endl;

    // compute weights for each point in output grid

    Eigen::SparseMatrix<double> W( nb_o_nodes, nb_i_nodes );

    WeightCache cache;
    std::string md5 = weights_hash(in_filename,clone_grid);
    bool wcached = cache.get( md5, W );
    if( ! wcached )
    {
        std::cout << ">>> computing weights ..." << std::endl;

        FEInterpolator interpolator;
        interpolator.compute_weights( *in_mesh, *out_mesh, W );
        cache.add( md5, W );
    }

    // interpolation -- multiply interpolant matrix with field vector

    std::cout << ">>> interpolating ..." << std::endl;

    {
        Timer t("interpolation");

        VectorXd::MapType fi = VectorXd::Map( &(ifield.data())[0], ifield.data().size() );
        VectorXd::MapType fo = VectorXd::Map( &(ofield.data())[0], ofield.data().size() );

        fo = W * fi;
    }

    // output to gmsh
    if(gmsh)
    {
        std::cout << ">>> output to gmsh" << std::endl;

        if(wcached)
            atlas::Tesselation::tesselate( *in_mesh );

        atlas::Gmsh::write3dsurf( *in_mesh, "input.msh" );

        atlas::Tesselation::tesselate( *out_mesh );

        atlas::Gmsh::write3dsurf( *out_mesh, std::string("output.msh") );
    }


    // output to grib

    {
        Timer t("grib write");
        std::cout << ">>> output to grib" << std::endl;
        GribWrite::clone(*out_mesh,clone_grid,out_filename);
    }
}

//------------------------------------------------------------------------------------------------------

int main( int argc, char **argv )
{
    MirInterpolate tool(argc,argv);
    tool.start();
    return 0;
}

