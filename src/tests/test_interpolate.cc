/*
 * (C) Copyright 1996-2012 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "atlas/grid/Field.h"
#include "atlas/grid/Grid.h"
#include "atlas/grid/LatLon.h"
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
    void test_values();
};

//-----------------------------------------------------------------------------

void TestInterpolate::test_constructor()
{
    // test the integrity of the objects not the values
    using namespace atlas::grid;

    BoundBox2D earth ( Point2(-90.,0.), Point2(90.,360.) );
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
    atlas::grid::Field::Data* raw_data = new atlas::grid::Field::Data;
    for (unsigned int i = 0; i <= 4; i++)
    {
        raw_data->push_back((double)i);
        raw_data->push_back((double)i);
        raw_data->push_back((double)i);
        raw_data->push_back((double)i);
        raw_data->push_back((double)i);
    }

    ASSERT(raw_data->size() == g->coordinates().size());

    const std::vector<Point2>& coords = g->coordinates();

    ASSERT( coords.size() == npts );

    atlas::grid::Field::MetaData* md = new atlas::grid::Field::MetaData();

    Field::Vector fv;
    atlas::grid::Field* f = new atlas::grid::Field(g, md, raw_data);
    fv.push_back(f);

    // check it's the same object
    ASSERT(&(f->metadata()) == md);

    // put it all into an INPUT field set
    atlas::grid::FieldSet input(fv);

    int count = 0;
    for (Field::Vector::iterator it = input.fields().begin(); it != input.fields().end(); ++it)
    {
        // test the data that is in the grid
        const Field::Data& d = (*it)->data();
        for (unsigned int i = 0; i < raw_data->size(); i++)
        {
            ASSERT((*raw_data)[i] == d[i]);
        }
    }

    // now assemble and output field set
    //
    Field::Vector fv1;
    int n1 = 4;
    Grid* g1 = new LatLon( n1, n1, earth );

    ASSERT( g1 );
    ASSERT( g1->coordinates().size() == (n1 + 1) * (n1 + 1) );

    atlas::grid::Field::MetaData* md1 = new atlas::grid::Field::MetaData();
    
    atlas::grid::Field* f1 = new atlas::grid::Field(g1, md1, new std::vector<double>);
    fv1.push_back(f1);

    // put it all into an OUTPUT field set
    atlas::grid::FieldSet output(fv1);
 
    // construct a bilinear interpolator
    mir::Interpolator interp;

    interp.interpolate(input, output);

    ASSERT(output.fields().size() == input.fields().size());
    
    count = 0;
    for (Field::Vector::iterator it = output.fields().begin(); it != output.fields().end(); ++it)
    {

        ASSERT(*it);

        const Field::Data& d = (*it)->data();
        // check the interpolator added data
        ASSERT(d.size() > 0);
    }

}

void TestInterpolate::test_values()
{
    using namespace atlas::grid;

    BoundBox2D earth ( Point2(-90.,0.), Point2(90.,360.) );

    // standard case
    const size_t n = 4;
    const size_t npts =  (n + 1) * (n + 1);

    Grid* g = new LatLon( n, n, earth );

    // make up some data for this field
    atlas::grid::Field::Data* raw_data = new atlas::grid::Field::Data;
    for (unsigned int i = 0; i <= 4; i++)
    {
        raw_data->push_back((double)i);
        raw_data->push_back((double)i);
        raw_data->push_back((double)i);
        raw_data->push_back((double)i);
        raw_data->push_back((double)i);
    }

    const std::vector<Point2>& coords = g->coordinates();

    atlas::grid::Field::MetaData* md = new atlas::grid::Field::MetaData();

    Field::Vector fv;
    atlas::grid::Field* f = new atlas::grid::Field(g, md, raw_data);
    fv.push_back(f);

    // put it all into an INPUT field set
    atlas::grid::FieldSet input(fv);

    // now assemble and output field set
    //
    Field::Vector fv1;
    // test the same sized grid - should get the same answer as the input grid
    const size_t m = n;
    Grid* g1 = new LatLon( m, m, earth );

    atlas::grid::Field::MetaData* m1 = new atlas::grid::Field::MetaData();
    
    atlas::grid::Field* f1 = new atlas::grid::Field(g1, m1, new std::vector<double>);
    fv1.push_back(f1);

    // put it all into an OUTPUT field set
    atlas::grid::FieldSet output(fv1);
 
    // construct a bilinear interpolator
    mir::Interpolator interp;

    interp.interpolate(input, output);

    for (Field::Vector::iterator it = output.fields().begin(); it != output.fields().end(); ++it)
    {
        const Field::Data& d = (*it)->data();
        
        // check the interpolator added data
        for (unsigned int i = 0; i < d.size(); i++)
        {
            /// @todo use standard epsilon value here
            std::cout << "value " << i << " compare " << d[i] << " with " << (*raw_data)[i] << std::endl;
            ASSERT(std::fabs(d[i] - (*raw_data)[i]) < 1.0e-10);
        }
    }

}

//-----------------------------------------------------------------------------

void TestInterpolate::run()
{
    test_constructor();
    test_values();
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

