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
#include "eckit/grib/GribHandle.h"

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

    Grid::Ptr make_grid( const std::string& filename );

public:

    MirInterpolate(int argc,char **argv): eckit::Tool(argc,argv)
    {
        gmsh = Resource<bool>("-gmsh",false);

        path_in = Resource<std::string>("-i","");
        if( path_in.asString().empty() )
            throw UserError( "missing input filename, parameter -i", Here());

        path_out = Resource<std::string>("-o","");
        if( path_out.asString().empty() )
            throw UserError( "missing output filename, parameter -o", Here());

        clone_path = Resource<std::string>("-g","");
        if( clone_path.asString().empty() )
            throw UserError( "missing clone grid filename, parameter -g", Here());

        method = Resource<std::string>("-m;$MIR_METHOD","fe");
    }

private:

    bool gmsh;

    PathName path_in;
    PathName path_out;
    PathName clone_path;

    std::string method;
};

//------------------------------------------------------------------------------------------------------

static GribAccessor<std::string> grib_shortName("shortName");

Grid::Ptr MirInterpolate::make_grid( const std::string& filename )
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

//------------------------------------------------------------------------------------------------------

void MirInterpolate::run()
{    
    std::cout.precision(std::numeric_limits< double >::digits10);
    std::cout << std::fixed;

    FieldSet::Ptr fs_inp;
    FieldSet::Ptr fs_out;

    // input grid + field

    std::cout << ">>> assembling input fields ..." << std::endl;

    fs_inp.reset( new FieldSet( path_in ) );
    if( fs_inp->empty() )
        throw UserError("Input fieldset is empty", Here());

// input to grib
//    {
//        GribWrite::clone( *fs_inp, path_in, path_in.asString() + ".input" );
//    }

    // output grid + field

    std::cout << ">>> assembling output fields ..." << std::endl;

    Grid::Ptr clone_grid = make_grid( clone_path );

    fs_out.reset( new FieldSet( clone_grid, fs_inp->field_names() ) );

    size_t npts_inp = fs_inp->grid().nPoints();
    size_t npts_out = fs_out->grid().nPoints();

    std::cout << "input  points " << npts_inp << std::endl;
    std::cout << "output points " << npts_out << std::endl;

    // compute weights for each point in output grid

    Eigen::SparseMatrix<double> W( npts_out, npts_inp );

    Weights* w;

    /// @todo make this into a factory
    if( method == std::string("fe") )
        w = new FiniteElement();
    if( method == std::string("kn") )
        w = new KNearest();

    if( !w )
        throw UserError( std::string("Unknown Interpolator type ") + method , Here() );

    w->assemble( fs_inp->grid(), fs_out->grid(), W );

    // interpolation -- multiply interpolant matrix with field vector

    size_t nfields = fs_inp->size();

    if( nfields != fs_out->size() )
        throw SeriousBug( "Number of fields in input does not match number of fields in ouput", Here() );

    std::cout << ">>> interpolating " << nfields << " fields ... " << std::endl;

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

    // output to gmsh

    if( gmsh )
    {
        std::cout << ">>> output to gmsh" << std::endl;

        Tesselation::tesselate( fs_inp->grid() );

        atlas::Gmsh::write3dsurf( fs_inp->grid().mesh(), "input.msh" );

        Tesselation::tesselate( fs_out->grid() );

        atlas::Gmsh::write3dsurf( fs_out->grid().mesh(), "output.msh" );
    }

    // output to grib

    {
        Timer t("grib output write");
        // GribWrite::write( *fs_out, path_out ); ///< @todo remove need for clone() with GridSpec's
        GribWrite::clone( *fs_out, clone_path, path_out );
    }
}

//------------------------------------------------------------------------------------------------------

int main( int argc, char **argv )
{
    MirInterpolate tool(argc,argv);
    tool.start();
    return 0;
}

