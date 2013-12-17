/*
 * (C) Copyright 1996-2013 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "eckit/log/Log.h"
#include "eckit/grid/Grid.h"
#include "eckit/grid/Field.h"
#include <string>
#include "Interpolator.h"
#include "PointSearch.h"

#include <Eigen/Sparse>
#include <Eigen/Dense>


using eckit::grid::Point2D;
using eckit::grid::Field;

//-----------------------------------------------------------------------------

namespace mir {

//-----------------------------------------------------------------------------

Interpolator::Interpolator(WeightEngine* engine /*=InverseSquare()*/)
: engine_(engine)
{
    ASSERT(engine_);
    eckit::Log::info() << "Build an Interpolator" << std::endl;
}

Interpolator::~Interpolator()
{
    eckit::Log::info() << "Destroy an Interpolator" << std::endl;
    delete engine_; engine_ = 0;
}

void Interpolator::interpolate(const eckit::grid::FieldSet& input, eckit::grid::FieldSet& output) const
{
    /// @todo refactor required here: most of this code is irrelevant to
    // the type of interpolation in place.
    //

    ASSERT( input.fields().size() == output.fields().size() );


    for( size_t i = 0; i < input.fields().size(); ++i )
    {
        ASSERT( input.fields()[i] );
        ASSERT( output.fields()[i] );

        const Field& inp = *(input.fields()[i]);
        Field& out = *(output.fields()[i]);

        /// @todo generate unique name from src grid and tgt grid and options
        ///       use here Grid::hash()
        std::string name = "todo";

        const size_t inp_npts = inp.grid().coordinates().size();
        const size_t out_npts = out.grid().coordinates().size();

        Eigen::SparseMatrix<double> W(out_npts, inp_npts);
        weights(inp, out, W);

        Eigen::MatrixXd A(inp_npts, 1);
        Eigen::MatrixXd B(1, out_npts);

        // get the input data into matrix A
        const Field::Data& fdata = inp.data();

        ASSERT(fdata.size() > 0);

        A.col(0) = Eigen::VectorXd::Map(&fdata[0], fdata.size());

        // interpolate
        B = W * A;

        out.data().resize( out_npts );

        // Write the output values from B to this vector
        Eigen::Map<Eigen::VectorXd>(out.data().data(), B.rows()) = B.col(0);
      

    }

}

void Interpolator::weights(const Field& inp, const Field& out, Eigen::SparseMatrix<double>& W) const
{
    const size_t inp_npts = inp.grid().coordinates().size();
    const size_t out_npts = out.grid().coordinates().size();
    
    W = Eigen::SparseMatrix<double>(out_npts, inp_npts);
    std::vector<Eigen::Triplet<double> > insertions;

    // 1. Find this set of weights in shared memory
    //
    // @todo

    // 2. If not found, generate the weights then store them in shmem with the above key
    //
    // for each point in the output grid, for bilinear we find the 4 nearest points in the
    // input grid and generate a set of weights from them

    const std::vector<Point2D>& out_coords = out.grid().coordinates();

    // set up an optimised point locator from the input grid
    PointSearch ps(inp.grid().coordinates());

    const size_t n = 4;

    // we should always have a means to generate weights
    ASSERT(engine_);

    // loop over the output grid points and find the closest ones
    std::vector<Point2D> closests;
    std::vector<unsigned int> indices;
    for (unsigned int i = 0; i < out_coords.size(); ++i)
    {
        const Point2D& o_pt = out_coords[i];

        // this function resizes the closests vector
        // @todo for bilinear interpolation we used to have edge cases
        // but for 3D searches we do not. However, we might need some
        // additional options here
        ps.closestNPoints(o_pt, n, closests, indices);

        ASSERT(closests.size() <= n);
        ASSERT(closests.size() == indices.size());

        std::vector<double> w;

        engine_->generate(o_pt, closests, w);    
        
        // write the locations in the IndexPoints to the matrix at location i
        for (unsigned int j = 0; j < w.size(); j++)
        {
            long pt_index = indices[j];

            ASSERT(w[j] <= 1.0);
            ASSERT(w[j] >= 0.0);
            insertions.push_back(Eigen::Triplet<double>(i, pt_index, w[j]));
        }

    }

    // set the weights from the triplets
    W.setFromTriplets(insertions.begin(), insertions.end());

}
} // namespace mir
