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
#include <algorithm>

#include <boost/progress.hpp>

#include "atlas/grid/ReducedGaussianGrid.h"
#include "atlas/grid/Tesselation.h"
#include "atlas/grid/Unstructured.h"
#include "atlas/util/ArrayView.hpp"

#include "eckit/config/Resource.h"
#include "eckit/log/Log.h"
#include "eckit/types/FloatCompare.h"
#include "eckit/utils/Translator.h"

#include "mir/Bilinear.h"
#include "mir/PointSearch.h"

//------------------------------------------------------------------------------------------------------

//#define assert assert

//#ifndef assert
//#define assert(x) ((void)sizeof(x))
//#endif

using Eigen::SparseMatrix;

using eckit::Resource;
using eckit::geometry::Point3;

using atlas::FunctionSpace;
using atlas::FieldT;


namespace mir {


//----------------------------------------------------------------------------eckit::geometry::Point3u--------------------------

Bilinear::Bilinear()
{
    nclosest_ = Resource<unsigned>( "Bilinear;$MIR_Bilinear", 4 );
    if( nclosest_ == 0 )
        throw eckit::UserError( "Bilinear k closest points cannot be 0", Here() );
}

Bilinear::~Bilinear()
{
}


bool eq(const double& a, const double& b)
{
    // @todo use the one in eckit once it stops giving you gip
    return fabs(a-b) < 10e-10;
}


bool left_right_lon_indexes(const double& in, atlas::ArrayView<double,2>& data, size_t start, size_t end, size_t& left, size_t& right)
{

    double right_lon = 360.0;
    double left_lon = 0.0;

    right = start; // take the first if there's a wrap
    left = start;

    for (unsigned int i = start; i < end; i++)
    {
        const double& val = data[i].data()[1];

        if (val < in || eq(val, in))
        {
            left_lon = val;
            left = i;
        }
        else
        {
            if (val < right_lon)
            {
                right_lon = val;
                right = i;
            }
        }
    }
}

void Bilinear::compute( Grid& in, Grid& out, Eigen::SparseMatrix<double>& W ) const
{
    std::cout << "Bilinear:: compute called " << std::endl;

    atlas::Mesh& i_mesh = in.mesh();
    atlas::Mesh& o_mesh = out.mesh();

    atlas::FunctionSpace& inodes = i_mesh.function_space( "nodes" );
    atlas::FunctionSpace& onodes = o_mesh.function_space( "nodes" );

    atlas::FieldT<double>& ilatlon = inodes.field<double>( "latlon" );
    atlas::FieldT<double>& olatlon = onodes.field<double>( "latlon" );

    atlas::ArrayView<double,2> icoords     ( ilatlon );
    atlas::ArrayView<double,2> ocoords     ( olatlon );

    atlas::grid::ReducedGaussianGrid* igg = dynamic_cast<atlas::grid::ReducedGaussianGrid*>(&in);

    /// @todo we only handle these at the moment
    if (!igg)
        return;

    // get the longitudes from
    const std::vector<long>& lons = igg->pointsPerLatitude();

    std::vector< Eigen::Triplet<double> > weights_triplets; /* structure to fill-in sparse matrix */

    // determing the number of output points required
    const size_t out_npts = onodes.boundsf()[1];
    weights_triplets.reserve( out_npts );

    for (unsigned int i = 0; i < out_npts; i++)
    {
        // get the lat, lon of the output data point required
        double lat = ocoords[i].data()[0];
        double lon = ocoords[i].data()[1];

        // these will hold indices in the input vector of the start of the upper and lower latitudes
        size_t top_i = 0;
        size_t bot_i = 0;

        // iterator over longitudes
        size_t ilon = 0;

        // we will need the number of points on the top and bottom latitudes later. store them
        size_t top_n, bot_n;

        for (unsigned int n = 0; n < lons.size() - 1; n++)
        {
            top_i = bot_i;
            bot_i += lons[n];

            top_n = lons[n];
            if ( (n + 1 ) == lons.size())
                bot_n = 0;
            else
                bot_n = lons[n+1];

            double top_lat = icoords[top_i].data()[0];

            double bot_lat = icoords[bot_i].data()[0];
            assert(top_lat != bot_lat);

            // check output point is on or below the hi latitude
            if (bot_lat < lat && (top_lat > lat || eq(top_lat, lat)))
            {
                assert(top_lat > lat || eq(top_lat, lat));
                assert(bot_lat < lat);
                assert(!eq(bot_lat, lat));

                break;
            }
        }

        double top_lat = icoords[top_i].data()[0];
        double bot_lat = icoords[bot_i].data()[0];
        assert(top_lat > lat || eq(top_lat, lat));
        assert(bot_lat < lat);
        assert(!eq(bot_lat, lat));


        // now get indeces to the left and right points on each of the data sectors
        // on the upper longitude
        size_t top_i_lft, top_i_rgt;

        left_right_lon_indexes(lon, icoords, top_i, top_i + top_n, top_i_lft, top_i_rgt);
        //left_right_lon_indexes(lon, icoords,  hi_data, tl, tr);
        assert(top_i_lft >= top_i);
        assert(top_i_lft < bot_i);
        assert(top_i_rgt >= top_i);
        assert(top_i_rgt < bot_i);
        assert(top_i_rgt != top_i_lft);

        // check the data is the same
        assert(eq(icoords[top_i_lft].data()[0],  icoords[top_i_rgt].data()[0]));

        // now get indeces to the left and right points on each of the data sectors
        // on the lower longitude
        size_t bot_i_lft, bot_i_rgt;

        size_t bot_i_end = bot_i + bot_n;

        left_right_lon_indexes(lon, icoords, bot_i, bot_i_end , bot_i_lft, bot_i_rgt);
        assert(bot_i_lft >= bot_i);
        assert(bot_i_lft < bot_i_end);
        assert(bot_i_rgt >= bot_i);
        assert(bot_i_rgt < bot_i_end);
        assert(bot_i_rgt != bot_i_lft);
        assert(eq(icoords[bot_i_lft].data()[0],  icoords[bot_i_rgt].data()[0]));

        // we now have the indices of tl, tr, bl, br points around the output point

        double tl_lon  = icoords[top_i_lft].data()[1];
        double tr_lon  = icoords[top_i_rgt].data()[1];
        double bl_lon  = icoords[bot_i_lft].data()[1];
        double br_lon  = icoords[bot_i_rgt].data()[1];

        // calculate the weights
        double w1 = (tl_lon - lon) / (tl_lon - tr_lon);
        double w2 = 1.0 - w1;
        double w3 = (bl_lon - lon) / (bl_lon - br_lon);
        double w4 = 1.0 - w3;

        // top and bottom midpoint weights
        double wt = (lat - bot_lat) / (top_lat - bot_lat);
        double wb = 1.0 - wt;

        // weights for the tl, tr, bl, br points
        double w_tl =  w2 * wt;
        double w_tr =  w1 * wt;
        double w_bl =  w4 * wb;
        double w_br =  w3 * wb;

        weights_triplets.push_back( Eigen::Triplet<double>( i, bot_i_rgt, w_br ) );
        weights_triplets.push_back( Eigen::Triplet<double>( i, bot_i_lft, w_bl ) );
        weights_triplets.push_back( Eigen::Triplet<double>( i, top_i_rgt, w_tr ) );
        weights_triplets.push_back( Eigen::Triplet<double>( i, top_i_lft, w_tl ) );

    }

    W.setFromTriplets(weights_triplets.begin(), weights_triplets.end());



}

std::string Bilinear::classname() const
{
    std::string ret ("Bilinear");
    ret += eckit::Translator<size_t,std::string>()(nclosest_);
    return ret;
}

//------------------------------------------------------------------------------------------------------

} // namespace mir

