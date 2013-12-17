/*
 * (C) Copyright 1996-2012 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "eckit/grid/Field.h"
#include "eckit/grid/Grid.h"
#include "eckit/grid/LatLon.h"
#include "eckit/log/Log.h"
#include "eckit/runtime/Tool.h"

#include "mir/Interpolator.h"

using namespace eckit;

//-----------------------------------------------------------------------------

namespace eckit_test {

//-----------------------------------------------------------------------------

class TestInterpolate : public Tool {
public:

    TestInterpolate(int argc,char **argv): Tool(argc,argv) {}

    ~TestInterpolate() {}

    virtual void run();

    void test_constructor();
};

//-----------------------------------------------------------------------------

void TestInterpolate::test_constructor()
{
    using namespace eckit::grid;

    BoundBox2D earth ( Point2D(-90.,0.), Point2D(90.,360.) );
    Grid* g = NULL;

    // standard case

    const size_t n = 4;
    const size_t npts =  (n + 1) * (n + 1);

    g = new LatLon( n, n, earth );

    ASSERT( g );
    ASSERT( g->coordinates().size() == npts );

    ASSERT( g->boundingBox().bottom_left_.lat_ == -90. );
    ASSERT( g->boundingBox().bottom_left_.lon_ ==   0. );
    ASSERT( g->boundingBox().top_right_.lat_ ==  90. );
    ASSERT( g->boundingBox().top_right_.lon_ == 360. );

    
    // make up some data for this field
    eckit::grid::Field::Data* raw_data = new eckit::grid::Field::Data;
    for (unsigned int i = 0; i <= 4; i++)
    {
        raw_data->push_back((double)i);
        raw_data->push_back((double)i);
        raw_data->push_back((double)i);
        raw_data->push_back((double)i);
        raw_data->push_back((double)i);
    }

    ASSERT(raw_data->size() == g->coordinates().size());

    const std::vector<Point2D>& coords = g->coordinates();

    ASSERT( coords.size() == npts );

    eckit::grid::Field::MetaData* m = new eckit::grid::Field::MetaData();

    Field::Vector fv;
    eckit::grid::Field* f = new eckit::grid::Field(g, m, raw_data);
    fv.push_back(f);

    // put it all into an INPUT field set
    eckit::grid::FieldSet input(fv);

    int count = 0;
    for (Field::Vector::iterator it = input.fields().begin(); it != input.fields().end(); ++it)
    {
        std::cout << "Input field " << count++ << std::endl;
        // test the data that is in the grid
        const Field::Data& d = (*it)->data();
        for (unsigned int i = 0; i < raw_data->size(); i++)
        {
            ASSERT((*raw_data)[i] == d[i]);
            std::cout << " input point " << i << " = " << d[i] << std::endl;
        }
    }

    // now assemble and output field set
    //
    Field::Vector fv1;
    int n1 = 8;
    Grid* g1 = new LatLon( n1, n1, earth );

    ASSERT( g1 );
    ASSERT( g1->coordinates().size() == (n1 + 1) * (n1 + 1) );

    eckit::grid::Field::MetaData* m1 = new eckit::grid::Field::MetaData();
    
    eckit::grid::Field* f1 = new eckit::grid::Field(g1, m1, new std::vector<double>);
    fv1.push_back(f1);

    // put it all into an OUTPUT field set
    eckit::grid::FieldSet output(fv1);
 
    // construct a bilinear interpolator
    mir::Interpolator interp;

    interp.interpolate(input, output);

    ASSERT(output.fields().size() == input.fields().size());
    
    count = 0;
    for (Field::Vector::iterator it = output.fields().begin(); it != output.fields().end(); ++it)
    {

        std::cout << "Output Field " << count++ << std::endl;
        // extract and test the data
        if (!*it)
            continue;

        const Field::Data& d = (*it)->data();
        
        // check the interpolator added data
        ASSERT(d.size() > 0);
        for (unsigned int i = 0; i < d.size(); i++)
        {
            /// @todo we need to test the output data here
           std::cout << "output point " << i << " = " << d[i] << std::endl;
        }
    }

}

//-----------------------------------------------------------------------------

void TestInterpolate::run()
{
    test_constructor();
}

//-----------------------------------------------------------------------------

} // namespace eckit_test

//-----------------------------------------------------------------------------

int main(int argc,char **argv)
{
    eckit_test::TestInterpolate mytest(argc,argv);
    mytest.start();
    return 0;
}

