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

Interpolator::Interpolator()
{
    eckit::Log::info() << "Build a Interpolator" << std::endl;
}

Interpolator::~Interpolator()
{
}

//-----------------------------------------------------------------------------
Bilinear::Bilinear() 
{
}

Bilinear::~Bilinear() 
{
}

double Bilinear::unnormalisedWeight(const Point2D& p, const Point2D& o) const
{
    return fabs(o.lat_ - p.lat_) * fabs(o.lon_ - p.lon_); 
}


void Bilinear::interpolate(const eckit::grid::FieldSet& input, eckit::grid::FieldSet& output)
{
    // @todo generate unique name from src grid and tgt grid and options
    std::string name = "todo"; 

    const int input_point_count = input.grid()->dataSize();    
    const int output_point_count = output.grid()->dataSize();    

    Eigen::SparseMatrix<double> weights = Eigen::SparseMatrix<double>(output_point_count, input_point_count);
    std::vector<Eigen::Triplet<double> > insertions;

    // 1. Find this set of weights in shared memory
    //
    // @todo 
    
    
    // 2. If not found, generate the weights then store them in shmem with the above key
    //
    // for each point in the output grid, for bilinear we find the 4 nearest points in the 
    // input grid and generate a set of weights from them

    const std::vector<Point2D>& outputGridData = output.grid()->gridData();

    // set up an optimised point locator from the input grid
    PointSearch ps(input.grid()->gridData());

    const size_t n = 4;
    
    // loop over the output grid points and find the closest ones
    std::vector<Point2D> closests; 
    std::vector<unsigned int> indices; 

    for (unsigned int i = 0; i < outputGridData.size(); ++i)
    {
        const Point2D& outputPoint = outputGridData[i];
        
        // this function resizes the closests vector
        // @todo for bilinear interpolation we used to have edge cases
        // but for 3D searches we do not. However, we might need some
        // additional options here 
        ps.closestNPoints(outputPoint, n, closests, indices);
        
        ASSERT(closests.size() <= n);
        ASSERT(closests.size() == indices.size());

        std::vector<double> weights(n, 0.0);  
        double sum = 0.0;
        
        for (unsigned int j = 0; j < closests.size(); j++)
        {
            // @todo this is not correct - we need to find the opposite
            // points to a particular point in order to determine its weight
            // -- this might form a requirement on the pointsearch class
            weights[j] = unnormalisedWeight(outputPoint, closests[j]);
            sum += weights[j];
        }
        
        // now normalise these
        for (unsigned int j = 0; j < closests.size(); j++)
        {
            if (sum != 0.0) 
                weights[j] /= sum;
        }
        
        // write the locations in the IndexPoints to the matrix at location i
        for (unsigned int j = 0; j < weights.size(); j++)
        {
            long pt_index = indices[j];
            
            insertions.push_back(Eigen::Triplet<double>(i, pt_index, weights[j]));
        } 

    }

    // set the weights from the triplets
    weights.setFromTriplets(insertions.begin(), insertions.end());

    Eigen::MatrixXd A(input_point_count, 1);
    Eigen::MatrixXd B(1, output_point_count);

    for (Field::Iterator it = input.fields().begin(); it != input.fields().end(); ++it)
    {
        // get the input data into matrix A
        const Field::Data* fdata = (*it)->data();

        A.col(0) = Eigen::VectorXd::Map(&(*fdata)[0], fdata->size());

        // interpolate
        B = weights * A;

        // @todo memory management. For now we new an object that the Field
        // class then owns
        std::vector<double>* output_values = new std::vector<double>(output_point_count);

        // Write the output values from B to this vector
        Eigen::Map<Eigen::VectorXd>(output_values->data(), B.rows()) = B.col(0);

        // add a field to the output fieldset
        
        // copy the metadata from input
        // @todo look at this again
        Field::MetaData* m = new Field::MetaData();
        *m = *(*it)->metadata();

        // create a new field and append it to the temporary field set
        Field* new_field = new Field(m, output_values);
        output.fields().push_back(new_field);
    }

 }
	
} // namespace mir
