#include <limits>
#include <cassert>
#include <sstream>
#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <memory>

#include <boost/progress.hpp>

#include "atlas/Gmsh.hpp"
#include "atlas/Mesh.hpp"

#include "eckit/exception/Exceptions.h"
#include "eckit/config/Resource.h"
#include "eckit/log/Timer.h"
#include "eckit/runtime/Tool.h"
#include "eckit/grib/GribAccessor.h"

#include "atlas/grid/GribRead.h"
#include "atlas/grid/GribWrite.h"
#include "atlas/grid/PointIndex3.h"
#include "atlas/grid/PointSet.h"
#include "atlas/grid/TriangleIntersection.h"
#include "atlas/grid/Tesselation.h"

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

#define DUMP_PROJ
#ifdef DUMP_PROJ
    static std::ofstream of("found.txt");
#endif

class FEInterpol {
public: // methods

    void compute_weights( atlas::Mesh& i_mesh, atlas::Mesh& o_mesh, Eigen::SparseMatrix<double>& W );

private: // methods

    bool project_point_to_triangle(KPoint3 &p, Vector3d& phi, int idx[3], const size_t k );

private: // members

    std::unique_ptr<PointIndex3> ptree;

    size_t ip_;

    size_t nb_triags;
    size_t inp_npts;

    FieldT<double>* picoords;
    FieldT<int>* ptriag_nodes;
};

bool FEInterpol::project_point_to_triangle(  KPoint3& p, Vector3d& phi, int idx[3], const size_t k )
{
    bool found = false;

    FieldT<int>& triag_nodes = *ptriag_nodes;
    FieldT<double>& icoords = *picoords;

    PointIndex3::NodeList cs = ptree->kNearestNeighbours(p,k);

#if 0
    std::cout << p << std::endl;
    for( size_t i = 0; i < cs.size(); ++i )
    {
        std::cout << cs[i] << std::endl;
    }
#endif

    // find in which triangle the point is contained
    // by computing the intercetion of the point with each nearest triangle

    Isect uvt;
    Ray ray( p.data() );

    size_t tid = std::numeric_limits<size_t>::max();

    for( size_t i = 0; i < cs.size(); ++i )
    {
        tid = cs[i].value().payload();

        KPoint3 tc = cs[i].value().point();

        ASSERT( tid < nb_triags );

        idx[0] = triag_nodes(0,tid);
        idx[1] = triag_nodes(1,tid);
        idx[2] = triag_nodes(2,tid);

        ASSERT( idx[0] < inp_npts && idx[1] < inp_npts && idx[2] < inp_npts );

        Triag triag( icoords.slice(idx[0]), icoords.slice(idx[1]), icoords.slice(idx[2]) );

        found = triag_intersection( triag, ray, uvt );

#ifdef DUMP_PROJ
        if(found)
            of << "[SUCCESS]" << std::endl;
//        else
//            of << "[FAILED]" << std::endl;

        if(found)
        of << "   i    " << i << std::endl
           << "   ip   " << ip_ << std::endl
           << "   p    " << p << std::endl
           << "   tc   " << tc << std::endl
           << "   d    " << KPoint3::distance(tc,p) << std::endl
           << "   tid  " << tid << std::endl
           << "   nidx " << idx[0] << " " << idx[1] << " " << idx[2] << std::endl
           << "   "
           << KPoint3(icoords.slice(idx[0])) << " / "
           << KPoint3(icoords.slice(idx[1])) << " / "
           << KPoint3(icoords.slice(idx[2])) << std::endl
           << "   uvwt " << uvt << std::endl;
#endif
        if(found) // weights are the baricentric cooridnates u,v
        {
            phi[0] = uvt.w();
            phi[1] = uvt.u;
            phi[2] = uvt.v;
            break;
        }

    } // loop over nearest triangles

    return found;
}

//------------------------------------------------------------------------------------------------------

// static size_t factorial[12] = { 1, 1, 2, 6, 24, 120, 720, 5040, 40320, 362880, 3628800, 39916800 };
static size_t factorial[10] = { 1, 1, 2, 6, 24, 120, 720, 5040, 40320, 362880 };

void FEInterpol::compute_weights( atlas::Mesh& i_mesh,
                                  atlas::Mesh& o_mesh,
                                  Eigen::SparseMatrix<double>& W )
{
    Timer t("compute weights");

    // generate mesh ...

    Tesselation::tesselate( i_mesh );

    // generate baricenters of each triangle & insert the baricenters on a kd-tree

    atlas::MeshGen::create_cell_centres( i_mesh );

    ptree.reset( create_cell_centre_index<PointIndex3>( i_mesh ) );

    // input mesh

    FunctionSpace&  i_nodes  = i_mesh.function_space( "nodes" );
    picoords = &i_nodes.field<double>( "coordinates" );

    FunctionSpace& triags = i_mesh.function_space( "triags" );

    ptriag_nodes = &triags.field<int>( "nodes" );

    nb_triags = triags.bounds()[1];
    inp_npts = i_nodes.bounds()[1];

    // output mesh

    FunctionSpace&  o_nodes  = o_mesh.function_space( "nodes" );
    FieldT<double>& ocoords  = o_nodes.field<double>( "coordinates" );

    const size_t out_npts = o_nodes.bounds()[1];

    // weights

    std::vector< Eigen::Triplet<double> > weights_triplets; /* structure to fill-in sparse matrix */

    weights_triplets.reserve( out_npts * 3 ); /* each row has 3 entries: one per vertice of triangle */

    /* search nearest k cell centres */

    boost::progress_display show_progress( out_npts );

    for( ip_ = 0; ip_ < out_npts; ++ip_ )
    {
        int idx[3]; /* indexes of the triangle that will contain the point*/
        Vector3d phi;
        KPoint3 p ( ocoords.slice(ip_) ); // lookup point

        size_t k = 1;
        while( ! project_point_to_triangle( p, phi, idx, factorial[k] ) )
        {
            ++k;
            if( k > (sizeof(factorial)/ sizeof(*factorial)) )
                throw eckit::TooManyRetries(k,"projecting point into tesselation");
        }

        ++show_progress;

        // insert the interpolant weights into the global (sparse) interpolant matrix

        for(int i = 0; i < 3; ++i)
            weights_triplets.push_back( Eigen::Triplet<double>( ip_, idx[i], phi[i] ) );
    }

    // fill-in sparse matrix

    W.setFromTriplets(weights_triplets.begin(), weights_triplets.end());
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
    grib_handle* h = grib_handle_new_from_file(0,fh,&err);

    if( h == 0 || err != 0 )
        throw ReadError( std::string("error reading grib file ") + fname );

    GribRead::read_nodes_from_grib( h, mesh );

    if( read_field )
        GribRead::read_field_from_grib( h, mesh, "field" );

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

    grib_load( in_filename, *in_mesh );

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

        FEInterpol interpol;
        interpol.compute_weights( *in_mesh, *out_mesh, W );
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
            Tesselation::tesselate( *in_mesh );

        atlas::Gmsh::write3dsurf( *in_mesh, "input.msh" );

        Tesselation::tesselate( *out_mesh );

        atlas::Gmsh::write3dsurf( *out_mesh, std::string("output.msh") );
    }


    // output to grib

    std::cout << ">>> output to grib" << std::endl;

    GribWrite::clone(*out_mesh,clone_grid,out_filename);

}

//------------------------------------------------------------------------------------------------------

int main( int argc, char **argv )
{
    MirInterpolate tool(argc,argv);
    tool.start();
    return 0;
}

